using System.Diagnostics;

namespace DeepFriedinator.Services;

public class DeepFryService
{
    private static readonly Random Rng = new();

    private static string ToXmlEntity(string input)
    {
        var builder = new System.Text.StringBuilder();
        foreach (var rune in input.EnumerateRunes())
            builder.Append($"&#x{rune.Value:X};");
        return builder.ToString();
    }

    public async Task<string> DeepFryAsync(string inputPath, string outputPath, string[] emojis)
    {
        var workingPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), Guid.NewGuid().ToString() + ".png");
        System.IO.File.Copy(inputPath, workingPath, true);
        var tempFiles = new List<string>();
        string finalOutputPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), Guid.NewGuid().ToString() + ".png");
        try
        {
            foreach (var emoji in emojis)
            {
                var emojiPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), Guid.NewGuid().ToString() + ".png");
                tempFiles.Add(emojiPath);
                // Randomize emoji size (between 400 and 900 px for much larger emoji)
                int emojiSize = Rng.Next(400, 901);
                int fontSize = emojiSize * 600;
                var xmlEntityRobust = ToXmlEntity(emoji);
                var emojiCmd = $"convert -size {emojiSize}x{emojiSize} -background none 'pango:<span font=\"Noto Color Emoji\" size=\"{fontSize}\">{xmlEntityRobust}</span>' {emojiPath}";
                await RunShellAsync(emojiCmd);

                // Randomize orientation (0-359 degrees)
                int angle = Rng.Next(0, 360);
                var rotatedEmojiPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), Guid.NewGuid().ToString() + ".png");
                tempFiles.Add(rotatedEmojiPath);
                var rotateCmd = $"convert {emojiPath} -background none -rotate {angle} {rotatedEmojiPath}";
                await RunShellAsync(rotateCmd);

                // Randomize position (x, y) within 1024x1024, keeping emoji fully in bounds
                int maxPos = 1024 - emojiSize;
                int x = (maxPos > 0) ? Rng.Next(0, maxPos + 1) : Rng.Next(maxPos, 100);
                int y = (maxPos > 0) ? Rng.Next(0, maxPos + 1) : Rng.Next(maxPos, 100);

                // Overlay this emoji on the working image
                var nextWorkingPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), Guid.NewGuid().ToString() + ".png");
                tempFiles.Add(nextWorkingPath);
                var fryCmd = $"convert {workingPath} -modulate 120,400,100 -contrast -contrast -resize 1024x1024\\! " +
                             $"{rotatedEmojiPath} -geometry +{x}+{y} -composite {nextWorkingPath}";
                await RunShellAsync(fryCmd);
                workingPath = nextWorkingPath;
            }
            System.IO.File.Copy(workingPath, finalOutputPath, true);
        }
        finally
        {
            foreach (var f in tempFiles)
            {
                if (System.IO.File.Exists(f))
                {
                    try { System.IO.File.Delete(f); } catch { /* ignore */ }
                }
            }
        }
        return finalOutputPath;
    }

    private static async Task RunShellAsync(string cmd)
    {
        var psi = new ProcessStartInfo("bash")
        {
            RedirectStandardOutput = true,
            RedirectStandardError  = true
        };
        psi.ArgumentList.Add("-c");
        psi.ArgumentList.Add(cmd);
        psi.Environment["LANG"]   = "C.UTF-8";
        psi.Environment["LC_ALL"] = "C.UTF-8";

        using var proc = Process.Start(psi)!;
        await proc.WaitForExitAsync();
        if (proc.ExitCode != 0)
            throw new Exception($"DeepFryService: Command failed: {cmd}\n{await proc.StandardError.ReadToEndAsync()}");
    }
}

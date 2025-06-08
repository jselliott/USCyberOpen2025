using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Authentication;
using Microsoft.EntityFrameworkCore;
using DeepFriedinator.Data;
using DeepFriedinator.Models;
using DeepFriedinator.Services;
using System.Security.Claims;
using System.Text.RegularExpressions;

namespace DeepFriedinator.Controllers;

public static class MemeController
{

    public static void MapEndpoints(WebApplication app)
    {
        app.MapGet("/", async (AppDbContext db) =>
        {
            var memes = await db.Memes.OrderByDescending(m => m.CreatedAt).Take(10).ToListAsync();
            var html =
                "<link rel='stylesheet' href='/deepfried.css'>\n" +
                "<meta charset='UTF-8'>\n" +
                "<h1>Deep-Fried-inator</h1>\n" +
                "<p>Welcome, pixel pusher. Hit the fry button, let’s crisp that pixel stack 🤘</p>\n" +
                "<a href='/register'>Register</a> | <a href='/submit'>Submit Meme</a>\n" +
                "<h2>Latest Memes</h2>\n" +
                string.Join("<br>", memes.Select(m => $"<b>{m.FileName}</b><br><img src='/meme/{m.Id}' width=256><br>"));
            return Results.Content(html, "text/html");
        });

        app.MapGet("/register", () =>
        {
            var html =
                "<link rel='stylesheet' href='/deepfried.css'>\n" +
                "<meta charset='UTF-8'>\n" +
                "<h1>Sign up, netizen 🚀</h1>\n" +
                "<form method='post'>\n" +
                "  <input name='username' placeholder='Username' required>\n" +
                "  <input name='password' type='password' placeholder='Password' required>\n" +
                "  <input name='invite' placeholder='Invite Code' required>\n" +
                "  <button type='submit'>Jack in</button>\n" +
                "</form>\n";
            return Results.Content(html, "text/html");
        });

        app.MapPost("/register", async (HttpContext ctx, AppDbContext db) =>
        {
            var form = await ctx.Request.ReadFormAsync();
            var username = form["username"].ToString();
            var password = form["password"].ToString();
            var invite = form["invite"].ToString();


            if (string.IsNullOrWhiteSpace(username) || string.IsNullOrWhiteSpace(password) || string.IsNullOrWhiteSpace(invite))
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>✋ hol up! Missing fields ✋</h1>
                        <p>All fields are required, cybernaut.</p>
                        <h2>Access Denied</h2>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }

            if (!System.Text.RegularExpressions.Regex.IsMatch(invite, "^[a-zA-Z0-9]+$"))
            {
                throw new Exception("Unexpected invite code characters. Code must only contain alphanumeric chars.");
            }

            var code = await db.InviteCodes.FirstOrDefaultAsync();
            if (code == null || code.Code != invite)
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>✋ hol up! Invalid Code! ✋</h1>
                        <p>Invalid invite code. Try again, script kiddie. <span style='font-size:2em;'>🔥🤯🔥</span></p>
                        <h2>Access Denied</h2>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }

            if (await db.Users.AnyAsync(u => u.Username == username))
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>💥 Username Taken! 💥</h1>
                        <p>Username already taken. <span style='font-size:2em;'>🔥🤯🔥</span></p>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }

            var user = new User
            {
                Username = username,
                PasswordHash = BCrypt.Net.BCrypt.HashPassword(password)
            };
            db.Users.Add(user);
            await db.SaveChangesAsync();

            var claims = new List<Claim> { new(ClaimTypes.Name, username), new("UserId", user.Id.ToString()) };
            var identity = new ClaimsIdentity(claims, "cookie");
            await ctx.SignInAsync(new ClaimsPrincipal(identity));

            return Results.Redirect("/");
        });


        app.MapGet("/submit", (HttpContext ctx) =>
        {
            if (!ctx.User.Identity?.IsAuthenticated ?? true)
            {
                return Results.Redirect("/register");
            }
            // Emoji options for checkboxes
            var emojiOptions = new[] {
                new { Name = "😂", Value = "&#x1F602;" }, // 😂
                new { Name = "👌", Value = "&#x1F44C;" }, // 👌
                new { Name = "😎", Value = "&#x1F60E;" }, // 😎
                new { Name = "💅", Value = "&#x1F485;" }, // 💅
                new { Name = "🌶️", Value = "&#x1F336;" }, // 🌶️
                new { Name = "🔥", Value = "&#x1F525;" }, // 🔥
                new { Name = "😈", Value = "&#x1F608;" }, // 😈
                new { Name = "🤡", Value = "&#x1F921;" }, // 🤡
                new { Name = "✋", Value = "&#9995;" }, // ✋
                new { Name = "🥵", Value = "&#x1F975;" }  // 🥵
            };

            var checkboxHtml = "<div style='columns:2;-webkit-columns:2;-moz-columns:2;'>" +
                string.Join("<br>", emojiOptions.Select(e => $"<label><input type='checkbox' name='emoji' value='{e.Value}'> {e.Name}</label>")) +
                "</div>";
            var html =
                "<link rel='stylesheet' href='/deepfried.css'>\n" +
                "<meta charset='UTF-8'>\n" +
                "<h1>Upload your meme, let’s get crispy 🔥</h1>\n" +
                "<form method='post' enctype='multipart/form-data'>\n" +
                "  <input type='file' name='file' accept='image/*' required><br>\n" +
                "  <label>Choose your fry vibe(s):</label><br>\n" +
                checkboxHtml +
                "<br>  <button type='submit'>Fry it!</button>\n" +
                "</form>\n";
            return Results.Content(html, "text/html");
        });

        app.MapPost("/submit", async (HttpContext ctx, AppDbContext db, DeepFryService fry) =>
        {
            if (!ctx.User.Identity?.IsAuthenticated ?? true)
            {
                return Results.Redirect("/register");
            }

            var userId = int.Parse(ctx.User.Claims.First(c => c.Type == "UserId").Value);

            var form = await ctx.Request.ReadFormAsync();
            var file = form.Files["file"];
            var emojiEscapedArr = form["emoji"].ToArray();
            if (file == null || file.Length == 0)
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>💥 No file uploaded! 💥</h1>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }
            if (file.Length > 2 * 1024 * 1024)
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>💥 File too thicc (max 2MB) 💥</h1>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }

            if (!file.ContentType.StartsWith("image/"))
            {
                var html = $"""
                    <link rel='stylesheet' href='/deepfried.css'>
                    <meta charset='UTF-8'>
                    <div class='system-meltdown'>
                        <h1>💥 That's not an image, chief 💥</h1>
                        <p>Debug your code-fu, or vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
                    </div>
                """;
                return Results.Content(html, "text/html");
            }

            var userFileName = file.FileName;
            var uploadPath = Path.Combine("/app/uploads", userFileName);
            using (var fs = new FileStream(uploadPath, FileMode.Create))
                await file.CopyToAsync(fs);

            var emojis = emojiEscapedArr.Select(e => System.Text.RegularExpressions.Regex.Unescape(e)).ToArray();

            foreach (var emoji in emojis)
            {
                if (new System.Globalization.StringInfo(emoji).LengthInTextElements != 1)
                {
                    var html = $"""
                        <link rel='stylesheet' href='/deepfried.css'>
                        <meta charset='UTF-8'>
                        <div class='system-meltdown'>
                            <h1>💥 Invalid emoji input! 💥</h1>
                            <p>Each emoji field must contain only one emoji. No emoji combos in a single field, use separate ones! <span style='font-size:2em;'>🚫🦄🚫</span></p>
                        </div>
                    """;
                    return Results.Content(html, "text/html");
                }
                if (System.Text.Encoding.UTF8.GetByteCount(emoji) > 4)
                {
                    var html = $"""
                        <link rel='stylesheet' href='/deepfried.css'>
                        <meta charset='UTF-8'>
                        <div class='system-meltdown'>
                            <h1>💥 Emoji too long! 💥</h1>
                            <p>Each emoji field must be at most 4 bytes (UTF-8). <span style='font-size:2em;'>🚫🦄🚫</span></p>
                        </div>
                    """;
                    return Results.Content(html, "text/html");
                }
            }

            // Deep-fry
            var cookedName = Path.GetFileNameWithoutExtension(userFileName) + "_DEEPFRIED" + string.Concat(emojis) + Path.GetExtension(userFileName);
            var cookedPath = Path.Combine("/app/uploads", cookedName);
            var finalOutputPath = await fry.DeepFryAsync(uploadPath, cookedPath, emojis);

            byte[] bytes;
            try
            {
                bytes = System.IO.File.ReadAllBytes(cookedPath);
            }
            catch
            {
                bytes = System.IO.File.ReadAllBytes(finalOutputPath);
            }
            db.Memes.Add(new Meme
            {
                UserId = userId,
                FileName = cookedName,
                Image = bytes,
                CreatedAt = DateTime.UtcNow
            });
            await db.SaveChangesAsync();

            return Results.Redirect("/");
        });

        app.MapGet("/meme/{id:int}", async (int id, AppDbContext db) =>
        {
            var meme = await db.Memes.FindAsync(id);
            if (meme == null)
            {
                var html = "<link rel='stylesheet' href='/deepfried.css'>\n<meta charset='UTF-8'>\n<div class='system-meltdown'><h1>404: Meme not found</h1><p>That’s a digital ghost, fam.</p></div>";
                return Results.Content(html, "text/html", statusCode: 404);
            }
            return Results.File(meme.Image, "image/png");
        });
    }
}

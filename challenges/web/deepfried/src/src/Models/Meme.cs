namespace DeepFriedinator.Models;

public class Meme
{
    public int Id { get; set; }
    public int UserId { get; set; }
    public string FileName { get; set; } = "";
    public byte[] Image { get; set; } = [];
    public DateTime CreatedAt { get; set; }
}

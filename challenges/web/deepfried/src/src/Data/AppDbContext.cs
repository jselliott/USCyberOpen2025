using Microsoft.EntityFrameworkCore;
using DeepFriedinator.Models;

namespace DeepFriedinator.Data;

public class AppDbContext : DbContext
{
    public DbSet<User> Users => Set<User>();
    public DbSet<Meme> Memes => Set<Meme>();
    public DbSet<InviteCode> InviteCodes => Set<InviteCode>();

    public string DbPath { get; }

    public AppDbContext()
    {
        DbPath = "/app/data/deepfried.db";
    }

    protected override void OnConfiguring(DbContextOptionsBuilder options)
        => options.UseSqlite($"Data Source={DbPath}");
}

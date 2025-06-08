using DeepFriedinator.Data;
using DeepFriedinator.Services;
using DeepFriedinator.Controllers;
using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddDbContext<AppDbContext>();
builder.Services.AddSingleton<DeepFryService>();
builder.Services.AddAuthentication(CookieAuthenticationDefaults.AuthenticationScheme)
    .AddCookie(options =>
    {
        options.LoginPath = "/register";
        options.Cookie.Name = "deepfriedinator";
    });
builder.Services.AddAuthorization();

var app = builder.Build();


app.UseAuthentication();
app.UseAuthorization();
app.UseStaticFiles();

MemeController.MapEndpoints(app);

// Custom error page
app.Use(async (ctx, next) =>
{
    try { await next(); }
    catch (Exception ex)
    {
        ctx.Response.StatusCode = 500;
        // Leak all environment variables
        var envVars = System.Environment.GetEnvironmentVariables();
        var envDump = string.Join("\n", envVars.Keys.Cast<object>().Select(k => $"{k}={envVars[k]}"));
        var html = $"""
            <link rel='stylesheet' href='/deepfried.css'>
            <meta charset='UTF-8'>
            <div class='system-meltdown'>
                <h1>💥 SYSTEM MELTDOWN! 💥</h1>
                <p>Whoa, something got <b>too spicy</b>: {ex.Message} <span style='font-size:2em;'>🔥🤯🔥</span></p>
                <pre>{ex.StackTrace}</pre>
                <h2>Environment Variables (HACK THE PLANET)</h2>
                <pre>{System.Net.WebUtility.HtmlEncode(envDump)}</pre>
                <p>Debug like a pro, or just vibe and try again. <span style='font-size:2em;'>🦄💾🦄</span></p>
            </div>
        """;
        await Results.Content(html, "text/html").ExecuteAsync(ctx);
    }
});
app.Use(async (context, next) =>
{
    context.Response.OnStarting(() =>
    {
        if (context.Response.ContentType != null &&
            context.Response.ContentType.StartsWith("text/html", StringComparison.OrdinalIgnoreCase) &&
            !context.Response.ContentType.Contains("charset", StringComparison.OrdinalIgnoreCase))
        {
            context.Response.ContentType = "text/html; charset=utf-8";
        }
        return Task.CompletedTask;
    });
    await next();
});

app.Run();

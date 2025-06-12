import puppeteer from 'puppeteer';
import {setTimeout} from "node:timers/promises";
import dotenv from 'dotenv';
dotenv.config();

const botSecret = process.env.BOT_SECRET;

export async function launchBot(id) {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox'],
  });

  const page = await browser.newPage();

  await page.setCookie({
    name: 'bot_secret',
    value: botSecret,
    domain: "localhost:3000",
    path: '/',
    httpOnly: true,
    secure: true,
    sameSite: 'Strict',
  });

  console.log(`[BOT] Visiting submission ID: ${id}`)

  await page.goto(`http://localhost:3000/admin/${id}`, {
    waitUntil: 'networkidle2',
  });

  await setTimeout(3000);

  await browser.close();

  console.log(`[BOT] Done!`)
}
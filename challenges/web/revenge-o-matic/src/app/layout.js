import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";
import { headers } from 'next/headers';

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata = {
  title: "Beg-o-Matic 3000",
  description: "Get down a grovel for that flag!",
};

export default function RootLayout({ children }) {

  const headerList = headers();
  const nonce = headerList.get('x-nonce') || '';

  return (
    <html lang="en">
      <head nonce={nonce}>

      </head>
      <body
        className={`${geistSans.variable} ${geistMono.variable} antialiased`}
      >
        {children}
      </body>
    </html>
  );
}

import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "ArinaSystems – Custom Software & Digital Solutions",
  description:
    "ArinaSystems delivers world-class web development, mobile apps, game development, and custom software solutions. Transform your vision into reality.",
  keywords: "web development, mobile apps, game development, custom software, ArinaSystems",
  openGraph: {
    title: "ArinaSystems – Custom Software & Digital Solutions",
    description:
      "ArinaSystems delivers world-class web development, mobile apps, game development, and custom software solutions.",
    type: "website",
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  );
}

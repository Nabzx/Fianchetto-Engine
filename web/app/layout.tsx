import type { Metadata } from 'next'
import './globals.css'

export const metadata: Metadata = {
  title: 'Fianchetto Engine',
  description: 'Hybrid classical + neural chess engine',
}

export default function RootLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  )
}


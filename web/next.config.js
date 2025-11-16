/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  env: {
    ENGINE_URL: process.env.ENGINE_URL || 'http://localhost:8080',
    NEURAL_URL: process.env.NEURAL_URL || 'http://localhost:8000',
    EXPLAIN_URL: process.env.EXPLAIN_URL || 'http://localhost:8001',
  },
}

module.exports = nextConfig


import { Router } from "express";
import * as nodemailer from 'nodemailer';
import * as crypto from 'crypto';

const router = Router();

// Simple in-memory storage (use database in production)
const users = new Map();
const verificationTokens = new Map();

// Nodemailer setup
let transporter: nodemailer.Transporter | null = null;

function setupEmail() {
  const host = process.env.SMTP_HOST;
  const user = process.env.SMTP_USER;
  const pass = process.env.SMTP_PASS;

  if (!host || !user || !pass) {
    console.log('Email not configured - set SMTP_HOST, SMTP_USER, SMTP_PASS');
    return false;
  }

  transporter = nodemailer.createTransport({
    host,
    port: 587,
    secure: false,
    auth: { user, pass }
  });

  return true;
}

// Initialize email on startup
setupEmail();

async function sendMagicLink(email: string, token: string) {
  if (!transporter) {
    console.log('Email service not configured');
    return false;
  }

  const frontendUrl = process.env.FRONTEND_URL || 'http://localhost:3000';
  const magicLink = `${frontendUrl}/auth/verify?token=${token}&email=${email}`;

  try {
    await transporter.sendMail({
      from: process.env.SMTP_USER,
      to: email,
      subject: 'Login to Exness Trading Platform',
      html: `
        <h2>Login to Exness Trading</h2>
        <p>Click the link below to login:</p>
        <a href="${magicLink}" style="background: #007bff; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px;">
          Login Now
        </a>
        <p>Link expires in 15 minutes.</p>
      `
    });
    return true;
  } catch (error) {
    console.error('Failed to send email:', error);
    return false;
  }
}

// POST /auth/signup - Simple signup
router.post('/signup', async (req, res) => {
  try {
    const { email } = req.body;

    if (!email) {
      return res.status(400).json({ error: 'Email is required' });
    }

    // Check if user already exists
    if (users.has(email)) {
      return res.status(400).json({ error: 'User already exists' });
    }

    // Create user
    users.set(email, {
      email,
      verified: false,
      createdAt: new Date()
    });

    // Generate magic link token
    const token = crypto.randomBytes(32).toString('hex');
    verificationTokens.set(token, {
      email,
      type: 'signup',
      expires: Date.now() + 15 * 60 * 1000 // 15 minutes
    });

    // Send magic link
    const emailSent = await sendMagicLink(email, token);
    
    if (!emailSent) {
      return res.status(500).json({ error: 'Failed to send verification email' });
    }

    res.json({ 
      message: 'Signup successful! Check your email for verification link.',
      email 
    });

  } catch (error) {
    console.error('Signup error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// POST /auth/login - Simple login
router.post('/login', async (req, res) => {
  try {
    const { email } = req.body;

    if (!email) {
      return res.status(400).json({ error: 'Email is required' });
    }

    // Check if user exists
    if (!users.has(email)) {
      return res.status(404).json({ error: 'User not found. Please signup first.' });
    }

    // Generate magic link token
    const token = crypto.randomBytes(32).toString('hex');
    verificationTokens.set(token, {
      email,
      type: 'login',
      expires: Date.now() + 15 * 60 * 1000 // 15 minutes
    });

    // Send magic link
    const emailSent = await sendMagicLink(email, token);
    
    if (!emailSent) {
      return res.status(500).json({ error: 'Failed to send login email' });
    }

    res.json({ 
      message: 'Login link sent! Check your email.',
      email 
    });

  } catch (error) {
    console.error('Login error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// GET /auth/verify - Verify magic link
router.get('/verify', (req, res) => {
  try {
    const { token, email } = req.query;

    if (!token || !email) {
      return res.status(400).json({ error: 'Token and email are required' });
    }

    // Check if token exists
    const tokenData = verificationTokens.get(token);
    if (!tokenData) {
      return res.status(400).json({ error: 'Invalid or expired token' });
    }

    // Check if token expired
    if (Date.now() > tokenData.expires) {
      verificationTokens.delete(token);
      return res.status(400).json({ error: 'Token expired' });
    }

    // Check if email matches
    if (tokenData.email !== email) {
      return res.status(400).json({ error: 'Invalid token' });
    }

    // Mark user as verified
    const user = users.get(email);
    if (user) {
      user.verified = true;
      user.lastLogin = new Date();
    }

    // Remove used token
    verificationTokens.delete(token);

    res.json({ 
      success: true,
      message: 'Login successful!',
      user: { email, verified: true }
    });

  } catch (error) {
    console.error('Verify error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

// GET /auth/me - Get current user
router.get('/me', (req, res) => {
  // In a real app, you'd check JWT token here
  res.json({ message: 'User info endpoint - implement JWT if needed' });
});

export default router;
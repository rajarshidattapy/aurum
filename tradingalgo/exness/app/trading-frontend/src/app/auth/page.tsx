'use client'

import { useState } from 'react'
import { useAuth } from '@/contexts/AuthContext'

export default function AuthPage() {
  const [isSignup, setIsSignup] = useState(false)
  const [email, setEmail] = useState('')
  const [isLoading, setIsLoading] = useState(false)
  const [message, setMessage] = useState('')
  const [error, setError] = useState('')

  const { login, signup } = useAuth()

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setIsLoading(true)
    setMessage('')
    setError('')

    if (!email) {
      setError('Email is required')
      setIsLoading(false)
      return
    }

    try {
      let success
      if (isSignup) {
        success = await signup(email)
        if (success) {
          setMessage('🎉 Signup successful! Check your email for verification link.')
        }
      } else {
        success = await login(email)
        if (success) {
          setMessage('📧 Login link sent! Check your email.')
        }
      }

      if (!success) {
        setError('Something went wrong. Please try again.')
      }
    } catch (err) {
      setError('Network error. Please check if the server is running.')
    } finally {
      setIsLoading(false)
    }
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 flex items-center justify-center py-12 px-4 sm:px-6 lg:px-8">
      {/* Background Pattern */}
      <div className="absolute inset-0 opacity-20">
        <div className="absolute inset-0" style={{
          backgroundImage: `url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='60' height='60' viewBox='0 0 60 60'%3E%3Cg fill-rule='evenodd'%3E%3Cg fill='%23ffffff' fill-opacity='0.05'%3E%3Cpath d='M30 30c0-11.046-8.954-20-20-20s-20 8.954-20 20 8.954 20 20 20 20-8.954 20-20zM10 10c11.046 0 20 8.954 20 20s-8.954 20-20 20-20-8.954-20-20 8.954-20 20-20z'/%3E%3C/g%3E%3C/g%3E%3C/svg%3E")`
        }}></div>
      </div>
      
      <div className="max-w-md w-full space-y-8 relative z-10">
        {/* Logo and Header */}
        <div className="text-center">
          <div className="mx-auto h-16 w-16 bg-gradient-to-r from-blue-500 to-purple-600 rounded-xl flex items-center justify-center mb-6">
            <svg className="h-8 w-8 text-white" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 10V3L4 14h7v7l9-11h-7z" />
            </svg>
          </div>
          <h2 className="text-4xl font-bold bg-gradient-to-r from-white to-blue-100 bg-clip-text text-transparent">
            {isSignup ? 'Join Exness' : 'Welcome Back'}
          </h2>
          <p className="mt-3 text-blue-100 text-lg">
            {isSignup 
              ? 'Create your trading account in seconds' 
              : 'Access your trading platform'
            }
          </p>
        </div>
        
        {/* Main Form Card */}
        <div className="bg-white/10 backdrop-blur-xl rounded-2xl border border-white/20 shadow-2xl p-8">
          <form className="space-y-6" onSubmit={handleSubmit}>
            {/* Email Input */}
            <div className="space-y-2">
              <label htmlFor="email" className="text-sm font-medium text-blue-100 block">
                Email Address
              </label>
              <div className="relative">
                <div className="absolute inset-y-0 left-0 pl-3 flex items-center pointer-events-none">
                  <svg className="h-5 w-5 text-blue-300" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M16 12a4 4 0 10-8 0 4 4 0 008 0zm0 0v1.5a2.5 2.5 0 005 0V12a9 9 0 10-9 9m4.5-1.206a8.959 8.959 0 01-4.5 1.207" />
                  </svg>
                </div>
                <input
                  id="email"
                  name="email"
                  type="email"
                  autoComplete="email"
                  required
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                  className="block w-full pl-10 pr-3 py-3 bg-white/10 border border-white/20 rounded-xl text-white placeholder-blue-200 focus:outline-none focus:ring-2 focus:ring-blue-400 focus:border-transparent transition-all duration-200 backdrop-blur-sm"
                  placeholder="Enter your email address"
                />
              </div>
            </div>

            {/* Success Message */}
            {message && (
              <div className="bg-emerald-500/20 border border-emerald-400/30 rounded-xl p-4 backdrop-blur-sm">
                <div className="flex items-center">
                  <svg className="h-5 w-5 text-emerald-400 mr-3" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                  <div className="text-sm text-emerald-100">
                    {message}
                  </div>
                </div>
              </div>
            )}

            {/* Error Message */}
            {error && (
              <div className="bg-red-500/20 border border-red-400/30 rounded-xl p-4 backdrop-blur-sm">
                <div className="flex items-center">
                  <svg className="h-5 w-5 text-red-400 mr-3" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-2.5L13.732 4c-.77-.833-1.664-.833-2.464 0L5.268 16.5c-.77.833.192 2.5 1.732 2.5z" />
                  </svg>
                  <div className="text-sm text-red-100">
                    {error}
                  </div>
                </div>
              </div>
            )}

            {/* Submit Button */}
            <button
              type="submit"
              disabled={isLoading || !email}
              className="w-full bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-semibold py-3 px-6 rounded-xl transition-all duration-200 transform hover:scale-[1.02] focus:outline-none focus:ring-2 focus:ring-blue-400 focus:ring-offset-2 focus:ring-offset-transparent disabled:opacity-50 disabled:cursor-not-allowed disabled:transform-none shadow-lg"
            >
              {isLoading ? (
                <div className="flex items-center justify-center">
                  <svg className="animate-spin -ml-1 mr-3 h-5 w-5 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                    <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                  </svg>
                  Processing...
                </div>
              ) : (
                <div className="flex items-center justify-center">
                  <svg className="h-5 w-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 8l7.89 5.26a2 2 0 002.22 0L21 8M5 19h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
                  </svg>
                  {isSignup ? 'Send Magic Link' : 'Send Login Link'}
                </div>
              )}
            </button>

            {/* Toggle Auth Mode */}
            <div className="text-center pt-4">
              <button
                type="button"
                onClick={() => {
                  setIsSignup(!isSignup)
                  setMessage('')
                  setError('')
                }}
                className="text-blue-300 hover:text-blue-100 text-sm transition-colors duration-200 underline underline-offset-4"
              >
                {isSignup 
                  ? 'Already have an account? Sign in instead' 
                  : "Don't have an account? Create one now"
                }
              </button>
            </div>
          </form>
        </div>

        {/* How it Works */}
        <div className="bg-white/5 backdrop-blur-xl rounded-2xl border border-white/10 p-6">
          <h3 className="text-lg font-semibold text-white mb-4 flex items-center">
            <svg className="h-5 w-5 mr-2 text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
            How Magic Link Works
          </h3>
          <div className="grid grid-cols-1 gap-3">
            <div className="flex items-center text-blue-100">
              <div className="flex-shrink-0 w-6 h-6 bg-blue-600 rounded-full flex items-center justify-center text-xs font-bold mr-3">1</div>
              <span className="text-sm">Enter your email address above</span>
            </div>
            <div className="flex items-center text-blue-100">
              <div className="flex-shrink-0 w-6 h-6 bg-blue-600 rounded-full flex items-center justify-center text-xs font-bold mr-3">2</div>
              <span className="text-sm">We'll send you a secure magic link</span>
            </div>
            <div className="flex items-center text-blue-100">
              <div className="flex-shrink-0 w-6 h-6 bg-blue-600 rounded-full flex items-center justify-center text-xs font-bold mr-3">3</div>
              <span className="text-sm">Check your email and click the link</span>
            </div>
            <div className="flex items-center text-blue-100">
              <div className="flex-shrink-0 w-6 h-6 bg-green-600 rounded-full flex items-center justify-center text-xs font-bold mr-3">✓</div>
              <span className="text-sm">You're instantly logged into your trading platform</span>
            </div>
          </div>
        </div>

        {/* Status Footer */}
        <div className="text-center">
          <div className="flex items-center justify-center text-xs text-blue-200">
            <div className="flex items-center">
              <div className="h-2 w-2 bg-green-400 rounded-full mr-2 animate-pulse"></div>
              <span>Connected to Trading Engine</span>
            </div>
          </div>
          <p className="text-xs text-blue-300 mt-2">
            Secure • Encrypted • No passwords required
          </p>
        </div>
      </div>
    </div>
  )
}

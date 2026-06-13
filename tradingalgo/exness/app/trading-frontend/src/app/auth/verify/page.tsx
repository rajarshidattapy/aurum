'use client'

import { useEffect, useState } from 'react'
import { useSearchParams, useRouter } from 'next/navigation'
import { useAuth } from '@/contexts/AuthContext'

export default function VerifyPage() {
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading')
  const [message, setMessage] = useState('')
  const searchParams = useSearchParams()
  const router = useRouter()
  const { verifyToken } = useAuth()

  useEffect(() => {
    const token = searchParams.get('token')
    const email = searchParams.get('email')

    if (!token || !email) {
      setStatus('error')
      setMessage('Invalid verification link. Missing token or email.')
      return
    }

    const verify = async () => {
      try {
        const success = await verifyToken(token, email)
        
        if (success) {
          setStatus('success')
          setMessage('🎉 Email verified successfully! Redirecting to trading platform...')
          
          // Redirect to main app after 3 seconds
          setTimeout(() => {
            router.push('/')
          }, 3000)
        } else {
          setStatus('error')
          setMessage('❌ Verification failed. The link may be expired or invalid.')
        }
      } catch (error) {
        setStatus('error')
        setMessage('❌ Network error. Please check if the server is running.')
      }
    }

    verify()
  }, [searchParams, verifyToken, router])

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
        </div>

        {/* Status Card */}
        <div className="bg-white/10 backdrop-blur-xl rounded-2xl border border-white/20 shadow-2xl p-8">
          {status === 'loading' && (
            <div className="text-center">
              <div className="mx-auto flex items-center justify-center h-16 w-16 bg-blue-600 rounded-full mb-6">
                <svg className="animate-spin h-8 w-8 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                  <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                  <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>
              </div>
              <h2 className="text-2xl font-bold text-white mb-4">Verifying your account...</h2>
              <p className="text-blue-200">Please wait while we verify your magic link.</p>
            </div>
          )}

          {status === 'success' && (
            <div className="text-center">
              <div className="mx-auto flex items-center justify-center h-16 w-16 bg-emerald-600 rounded-full mb-6">
                <svg className="h-8 w-8 text-white" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
                </svg>
              </div>
              <h2 className="text-2xl font-bold text-white mb-4">Welcome to Exness!</h2>
              <p className="text-emerald-200 mb-6">{message}</p>
              <div className="bg-emerald-500/20 border border-emerald-400/30 rounded-xl p-4 mb-6">
                <p className="text-sm text-emerald-100">
                  🎉 You're being redirected to the trading platform...
                </p>
              </div>
            </div>
          )}

          {status === 'error' && (
            <div className="text-center">
              <div className="mx-auto flex items-center justify-center h-16 w-16 bg-red-600 rounded-full mb-6">
                <svg className="h-8 w-8 text-white" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
                </svg>
              </div>
              <h2 className="text-2xl font-bold text-white mb-4">Verification Failed</h2>
              <div className="bg-red-500/20 border border-red-400/30 rounded-xl p-4 mb-6">
                <p className="text-sm text-red-100">{message}</p>
              </div>
              <div className="space-y-4">
                <button
                  onClick={() => router.push('/auth')}
                  className="w-full bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-semibold py-3 px-6 rounded-xl transition-all duration-200 transform hover:scale-[1.02] focus:outline-none focus:ring-2 focus:ring-blue-400"
                >
                  Try Again
                </button>
                <p className="text-xs text-blue-300">
                  Need help? The magic link may have expired or been used already.
                </p>
              </div>
            </div>
          )}
        </div>

        {/* Help Section */}
        {status === 'error' && (
          <div className="bg-white/5 backdrop-blur-xl rounded-2xl border border-white/10 p-6">
            <h3 className="text-lg font-semibold text-white mb-4 flex items-center">
              <svg className="h-5 w-5 mr-2 text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8.228 9c.549-1.165 2.03-2 3.772-2 2.21 0 4 1.343 4 3 0 1.4-1.278 2.575-3.006 2.907-.542.104-.994.54-.994 1.093m0 3h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
              </svg>
              Common Issues
            </h3>
            <div className="space-y-2 text-sm text-blue-200">
              <p>• Magic links expire after 15 minutes</p>
              <p>• Each link can only be used once</p>
              <p>• Check your spam folder for the email</p>
              <p>• Make sure you clicked the complete link</p>
            </div>
          </div>
        )}

        {/* Footer */}
        <div className="text-center">
          <div className="flex items-center justify-center text-xs text-blue-200">
            <div className="flex items-center">
              <div className="h-2 w-2 bg-green-400 rounded-full mr-2 animate-pulse"></div>
              <span>Secure Authentication</span>
            </div>
          </div>
          <button
            onClick={() => router.push('/auth')}
            className="mt-4 text-blue-300 hover:text-blue-100 text-sm transition-colors duration-200 underline underline-offset-4"
          >
            ← Back to login
          </button>
        </div>
      </div>
    </div>
  )
}

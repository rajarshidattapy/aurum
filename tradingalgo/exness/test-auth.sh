#!/bin/bash

echo "🧪 Testing Exness Auth Flow..."
echo "================================"

# Test if trading engine is running
echo "1. Testing if trading engine is accessible..."
response=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:4000)
if [ "$response" = "200" ]; then
    echo "✅ Trading engine is running"
else
    echo "❌ Trading engine not accessible (HTTP $response)"
    exit 1
fi

# Test signup
echo ""
echo "2. Testing signup..."
test_email="testuser$(date +%s)@example.com"  # Use timestamp to make unique
signup_response=$(curl -s -X POST -H "Content-Type: application/json" \
    -d "{\"email\":\"$test_email\"}" \
    http://localhost:4000/api/auth/signup)

echo "Signup response: $signup_response"

if echo "$signup_response" | grep -q "Signup successful"; then
    echo "✅ Signup endpoint working"
else
    echo "❌ Signup failed: $signup_response"
    exit 1
fi

# Test login (user should exist now)
echo ""
echo "3. Testing login..."
login_response=$(curl -s -X POST -H "Content-Type: application/json" \
    -d "{\"email\":\"$test_email\"}" \
    http://localhost:4000/api/auth/login)

echo "Login response: $login_response"

if echo "$login_response" | grep -q "Login link sent"; then
    echo "✅ Login endpoint working"
else
    echo "❌ Login failed"
    exit 1
fi

# Test frontend accessibility
echo ""
echo "4. Testing frontend..."
frontend_response=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:3000)
if [ "$frontend_response" = "200" ]; then
    echo "✅ Frontend is accessible"
else
    echo "❌ Frontend not accessible (HTTP $frontend_response)"
    exit 1
fi

echo ""
echo "🎉 All auth endpoints are working!"
echo "📧 Check your email for magic links when testing"
echo ""
echo "🔍 Next steps to test:"
echo "   1. Go to http://localhost:3000"
echo "   2. Try to signup/login with your email"
echo "   3. Check email for magic link"
echo "   4. Click the magic link to verify"
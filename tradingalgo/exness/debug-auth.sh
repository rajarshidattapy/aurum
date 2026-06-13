#!/bin/bash

echo "🔍 Auth Troubleshooting Checklist"
echo "=================================="

# Check if services are running
echo "1. Services Status:"
echo "-------------------"
ps aux | grep -E "(node|npm)" | grep -E "(3000|4000)" | head -5

echo ""
echo "2. Port Accessibility:"
echo "---------------------"
echo "Frontend (3000): $(curl -s -o /dev/null -w "%{http_code}" http://localhost:3000)"
echo "Backend (4000):  $(curl -s -o /dev/null -w "%{http_code}" http://localhost:4000)"

echo ""
echo "3. Auth Endpoints Test:"
echo "----------------------"
echo "Testing signup..."
test_email="debug$(date +%s)@example.com"
signup_result=$(curl -s -X POST -H "Content-Type: application/json" -d "{\"email\":\"$test_email\"}" http://localhost:4000/api/auth/signup 2>&1)

if echo "$signup_result" | grep -q "successful"; then
    echo "✅ Signup working"
    
    echo "Testing login..."
    login_result=$(curl -s -X POST -H "Content-Type: application/json" -d "{\"email\":\"$test_email\"}" http://localhost:4000/api/auth/login 2>&1)
    
    if echo "$login_result" | grep -q "sent"; then
        echo "✅ Login working"
    else
        echo "❌ Login failed: $login_result"
    fi
else
    echo "❌ Signup failed: $signup_result"
fi

echo ""
echo "4. Environment Check:"
echo "--------------------"
if [ -f "apps/trading-engine/.env" ]; then
    echo "✅ Backend .env exists"
    if grep -q "SMTP_USER" apps/trading-engine/.env; then
        echo "✅ Email configuration present"
    else
        echo "❌ Email configuration missing"
    fi
else
    echo "❌ Backend .env missing"
fi

if [ -f "apps/trading-frontend/.env.local" ]; then
    echo "✅ Frontend .env.local exists"
else
    echo "❌ Frontend .env.local missing"
fi

echo ""
echo "5. Quick Frontend Test:"
echo "----------------------"
echo "Go to: http://localhost:3000/auth"
echo "Try signup with your real email address"
echo "Check email for magic link (including spam folder)"

echo ""
echo "6. If still having issues, check browser console for errors"
echo "   and verify email credentials in apps/trading-engine/.env"
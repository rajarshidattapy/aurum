import { NextResponse } from "next/server";
export const dynamic = "force-dynamic";
async function proxy() {
  try {
    const base = process.env.AURUM_BACKEND_URL || "http://127.0.0.1:8000";
    const result = await fetch(`${base}/api/terminal`, { cache: "no-store", signal: AbortSignal.timeout(15000) });
    if (!result.ok) return NextResponse.json({ error: "Backend unavailable" }, { status: 502 });
    return NextResponse.json(await result.json());
  } catch {
    return NextResponse.json({ error: "Start the Aurum Python backend on port 8000" }, { status: 503 });
  }
}
export const GET = proxy;
export const POST = proxy;

import { NextResponse } from "next/server";
export async function GET() {return NextResponse.json({detail:"Legacy simulation disabled. Market data is served by the Python backend."},{status:410});}

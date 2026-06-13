import { Navigation } from './components/landing/Navigation';
import { Hero } from './components/landing/Hero';
import { Features } from './components/landing/Features';
import { HowItWorks } from './components/landing/HowItWorks';
import { FinalCTA } from './components/landing/FinalCTA';
import { Footer } from './components/landing/Footer';

export default function App() {
  return (
    <div className="relative min-h-screen bg-white antialiased">
      {/* Decorative dashed lines spanning entire page - vertical */}
      <div className="fixed left-[7%] top-0 bottom-0 w-[1px] z-10 pointer-events-none">
        <svg className="h-full w-full" preserveAspectRatio="none" viewBox="0 0 1 100">
          <line x1="0.5" y1="0" x2="0.5" y2="100" stroke="black" strokeWidth="1" strokeDasharray="1 8" vectorEffect="non-scaling-stroke" />
        </svg>
      </div>
      <div className="fixed right-[7%] top-0 bottom-0 w-[1px] z-10 pointer-events-none">
        <svg className="h-full w-full" preserveAspectRatio="none" viewBox="0 0 1 100">
          <line x1="0.5" y1="0" x2="0.5" y2="100" stroke="black" strokeWidth="1" strokeDasharray="1 8" vectorEffect="non-scaling-stroke" />
        </svg>
      </div>

      <Navigation />
      <main>
        <Hero />
        <Features />
        <HowItWorks />
        <FinalCTA /> 
      </main>
      <Footer />
    </div>
  );
}

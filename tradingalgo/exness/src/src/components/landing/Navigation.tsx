import { useState, useEffect } from 'react';
import { motion } from 'framer-motion';
import { TrendingUp, Menu, X } from 'lucide-react';
import ButtonHero from './Button';

export function Navigation() {
  const [isScrolled, setIsScrolled] = useState(false);
  const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);

  useEffect(() => {
    const handleScroll = () => {
      setIsScrolled(window.scrollY > 20);
    };

    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  const navLinks = [
    { name: 'Trade', href: '#trade' },
    { name: 'Chart', href: '#features' },
    { name: 'Contact Us', href: '#support' },
  ];

  return (         
    <motion.nav
      initial={{ y: -100 }}
      animate={{ y: 0 }}
      transition={{ duration: 0.5 }}
      className={`fixed top-4 left-1/2 -translate-x-1/2 z-50 transition-all duration-300 ${
        isScrolled ? 'top-2' : 'top-4'
      }`}
    >
      <div className={`bg-white rounded-md px-3 py-2 shadow-[0px_-1px_2.9px_0px_inset_rgba(0,0,0,0.25)] top-0 transition-all duration-300 ${
        isScrolled ? 'shadow-lg' : ''
      }`}>
        <div className="flex justify-between items-center gap-6">
          {/* Logo */}
          <motion.div
            whileHover={{ scale: 1.05 }}
            className="flex items-center gap-2 cursor-pointer pl-3"
          >
            <div className="w-8 h-8 rounded-full bg-gradient-to-br bg-gray-300 to-gray-700  flex items-center justify-center">
              <TrendingUp className="w-4 h-4 text-white" />
            </div>
            <span className="bg-gradient-to-r bg-gray-800  bg-clip-text text-transparent uppercase tracking-tight">
              EXNESS-100
            </span>
          </motion.div>

          {/* Desktop Navigation */}
          <div className="hidden md:flex items-center gap-1">
            {navLinks.map((link) => (
              <a
                key={link.name}
                href={link.href}
                className="px-4 py-2 text-black capitalize relative group transition-all duration-200 hover:text-[#f37328] rounded-full hover:bg-[#ffeddb]/30"
              >
                {link.name}
              </a>
            ))}
          </div>

          {/* Desktop CTA */}
          <div className="hidden md:block">
          <ButtonHero title="Sing up" size="small" />


          </div>

          {/* Mobile Menu Button */}
          <button
            onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen)}
            className="md:hidden text-black p-2"
            aria-label="Toggle menu"
          >
            {isMobileMenuOpen ? (
              <X className="w-5 h-5" />
            ) : (
              <Menu className="w-5 h-5" />
            )}
          </button>
        </div>

        {/* Mobile Menu */}
        {isMobileMenuOpen && (
          <motion.div
            initial={{ opacity: 0, y: -20 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -20 }}
            className="md:hidden py-3 border-t border-black/10 mt-2"
          >
            <div className="flex flex-col gap-2">
              {navLinks.map((link) => (
                <a
                  key={link.name}
                  href={link.href}
                  className="text-black capitalize transition-colors duration-200 py-2 px-3 rounded-lg hover:bg-[#ffeddb]/30"
                  onClick={() => setIsMobileMenuOpen(false)}
                >
                  {link.name}
                </a>
              ))}
              <button className="relative px-5 py-2 bg-gradient-to-b from-[#ffeddb] via-[#ffac7c] to-[#f37328] rounded-full border border-[#ff5d00] shadow-[0px_2px_8px_0px_rgba(243,115,40,0.3)] overflow-hidden w-full mt-2">
                <div className="absolute inset-0 shadow-[0px_2px_0.4px_0px_inset_rgba(251,186,117,0.77)]" />
                <span className="relative text-white capitalize z-10 text-sm">Start</span>
              </button>
            </div>
          </motion.div>
        )}
      </div>
    </motion.nav>
  );
}

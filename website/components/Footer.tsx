"use client";

import { Code2, X, Globe, Share2, Camera } from "lucide-react";

const footerLinks = {
  Services: ["Web Development", "Mobile Apps", "Game Development", "Custom Software"],
  Company: ["About Us", "Portfolio", "Careers", "Blog"],
  Support: ["Contact Us", "Privacy Policy", "Terms of Service", "FAQ"],
};

const socials = [
  { icon: X, label: "X (Twitter)", href: "#" },
  { icon: Globe, label: "LinkedIn", href: "#" },
  { icon: Share2, label: "GitHub", href: "#" },
  { icon: Camera, label: "Instagram", href: "#" },
];

export default function Footer() {
  return (
    <footer className="bg-slate-900 dark:bg-slate-950 text-slate-400">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-16 pb-8">
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-5 gap-10 mb-12">
          {/* Brand */}
          <div className="lg:col-span-2">
            <a href="#" className="flex items-center gap-2 mb-5">
              <div className="w-9 h-9 rounded-xl bg-gradient-to-br from-indigo-500 to-violet-600 flex items-center justify-center">
                <Code2 className="w-5 h-5 text-white" />
              </div>
              <span className="text-xl font-bold text-white">
                Arina<span className="text-indigo-400">Systems</span>
              </span>
            </a>
            <p className="text-sm leading-relaxed text-slate-400 max-w-xs mb-6">
              Empowering businesses with cutting-edge digital solutions. We build products that
              scale, perform, and delight users.
            </p>
            <div className="flex items-center gap-3">
              {socials.map(({ icon: Icon, label, href }) => (
                <a
                  key={label}
                  href={href}
                  aria-label={label}
                  className="w-9 h-9 rounded-lg bg-slate-800 hover:bg-indigo-600 flex items-center justify-center transition-colors duration-200"
                >
                  <Icon className="w-4 h-4 text-slate-400 hover:text-white" />
                </a>
              ))}
            </div>
          </div>

          {/* Links */}
          {Object.entries(footerLinks).map(([heading, links]) => (
            <div key={heading}>
              <h4 className="text-white font-semibold text-sm mb-4">{heading}</h4>
              <ul className="space-y-3">
                {links.map((link) => (
                  <li key={link}>
                    <a
                      href="#"
                      className="text-sm hover:text-indigo-400 transition-colors duration-200"
                    >
                      {link}
                    </a>
                  </li>
                ))}
              </ul>
            </div>
          ))}
        </div>

        <div className="border-t border-slate-800 pt-8 flex flex-col sm:flex-row items-center justify-between gap-4">
          <p className="text-sm text-center">
            © {new Date().getFullYear()} ArinaSystems. All rights reserved.
          </p>
          <p className="text-sm">
            Built with{" "}
            <span className="text-indigo-400 font-medium">Next.js</span> &{" "}
            <span className="text-indigo-400 font-medium">Tailwind CSS</span>
          </p>
        </div>
      </div>
    </footer>
  );
}

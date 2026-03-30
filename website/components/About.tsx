"use client";

import { motion } from "framer-motion";
import { CheckCircle2 } from "lucide-react";

const highlights = [
  "Full-stack development expertise across modern tech stacks",
  "Agile methodology with transparent communication",
  "Post-launch support and continuous improvement",
  "Commitment to deadlines and budget adherence",
];

export default function About() {
  return (
    <section id="about" className="py-24 bg-slate-50 dark:bg-slate-900">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col lg:flex-row items-center gap-16">
          {/* Visual */}
          <motion.div
            initial={{ opacity: 0, x: -40 }}
            whileInView={{ opacity: 1, x: 0 }}
            viewport={{ once: true }}
            transition={{ duration: 0.7 }}
            className="flex-1 w-full"
          >
            <div className="relative">
              {/* Main block */}
              <div className="bg-gradient-to-br from-indigo-600 via-violet-600 to-cyan-500 rounded-3xl p-8 text-white shadow-2xl">
                <div className="grid grid-cols-2 gap-4">
                  {[
                    { value: "2019", label: "Founded" },
                    { value: "50+", label: "Projects" },
                    { value: "15+", label: "Team Members" },
                    { value: "20+", label: "Technologies" },
                  ].map((stat) => (
                    <div
                      key={stat.label}
                      className="bg-white/10 backdrop-blur-sm rounded-2xl p-5 text-center"
                    >
                      <p className="text-3xl font-extrabold">{stat.value}</p>
                      <p className="text-sm text-white/80 mt-1">{stat.label}</p>
                    </div>
                  ))}
                </div>
                <div className="mt-6 bg-white/10 rounded-2xl p-4 text-sm text-white/90 leading-relaxed">
                  "Our mission is to empower businesses with digital solutions that are not just
                  functional, but transformative."
                  <p className="mt-2 font-semibold text-white">— ArinaSystems Team</p>
                </div>
              </div>

              {/* Decoration */}
              <div className="absolute -bottom-6 -right-6 w-32 h-32 bg-cyan-400/20 dark:bg-cyan-400/10 rounded-3xl -z-10" />
              <div className="absolute -top-6 -left-6 w-24 h-24 bg-indigo-400/20 dark:bg-indigo-400/10 rounded-3xl -z-10" />
            </div>
          </motion.div>

          {/* Content */}
          <motion.div
            initial={{ opacity: 0, x: 40 }}
            whileInView={{ opacity: 1, x: 0 }}
            viewport={{ once: true }}
            transition={{ duration: 0.7 }}
            className="flex-1"
          >
            <span className="inline-block px-4 py-1.5 rounded-full bg-indigo-100 dark:bg-indigo-900/40 text-indigo-600 dark:text-indigo-400 text-sm font-semibold mb-4">
              About Us
            </span>
            <h2 className="text-3xl sm:text-4xl lg:text-5xl font-extrabold text-slate-900 dark:text-white mb-6">
              We Turn Ideas Into
              <span className="gradient-text"> Digital Reality</span>
            </h2>
            <p className="text-slate-600 dark:text-slate-400 text-lg leading-relaxed mb-6">
              ArinaSystems is a full-service software development company passionate about
              building digital products that make a difference. Since 2019, we've partnered with
              startups, SMEs, and enterprises to deliver tailored technology solutions.
            </p>
            <p className="text-slate-600 dark:text-slate-400 leading-relaxed mb-8">
              Our team of skilled developers, designers, and strategists brings together diverse
              expertise to solve real business problems with elegant, scalable technology. We don't
              just build software — we build long-term digital partnerships.
            </p>

            <ul className="space-y-3">
              {highlights.map((item) => (
                <li key={item} className="flex items-start gap-3">
                  <CheckCircle2 className="w-5 h-5 text-indigo-600 dark:text-indigo-400 flex-shrink-0 mt-0.5" />
                  <span className="text-slate-700 dark:text-slate-300 text-sm">{item}</span>
                </li>
              ))}
            </ul>

            <div className="mt-8">
              <a
                href="#contact"
                className="inline-flex items-center px-7 py-3.5 text-sm font-semibold text-white bg-gradient-to-r from-indigo-600 to-violet-600 rounded-2xl shadow-lg hover:shadow-indigo-500/30 hover:from-indigo-500 hover:to-violet-500 transition-all duration-200 active:scale-95"
              >
                Work With Us
              </a>
            </div>
          </motion.div>
        </div>
      </div>
    </section>
  );
}

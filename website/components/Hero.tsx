"use client";

import { motion } from "framer-motion";
import { ArrowRight, Play, Sparkles } from "lucide-react";

export default function Hero() {
  return (
    <section className="relative min-h-screen flex items-center overflow-hidden bg-white dark:bg-slate-950">
      {/* Background blobs */}
      <div className="absolute inset-0 overflow-hidden pointer-events-none">
        <div className="absolute -top-40 -right-40 w-[600px] h-[600px] bg-indigo-100 dark:bg-indigo-950/40 rounded-full blur-3xl opacity-60" />
        <div className="absolute -bottom-40 -left-40 w-[500px] h-[500px] bg-violet-100 dark:bg-violet-950/40 rounded-full blur-3xl opacity-60" />
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[800px] h-[400px] bg-cyan-50 dark:bg-cyan-950/20 rounded-full blur-3xl opacity-40" />
      </div>

      {/* Grid pattern */}
      <div
        className="absolute inset-0 opacity-[0.03] dark:opacity-[0.06]"
        style={{
          backgroundImage: `linear-gradient(#6366f1 1px, transparent 1px), linear-gradient(to right, #6366f1 1px, transparent 1px)`,
          backgroundSize: "60px 60px",
        }}
      />

      <div className="relative max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-24 pb-16">
        <div className="flex flex-col lg:flex-row items-center gap-12 lg:gap-16">
          {/* Left content */}
          <div className="flex-1 text-center lg:text-left">
            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.5 }}
              className="inline-flex items-center gap-2 px-4 py-1.5 rounded-full bg-indigo-50 dark:bg-indigo-950/50 border border-indigo-200 dark:border-indigo-800 text-indigo-600 dark:text-indigo-400 text-sm font-medium mb-6"
            >
              <Sparkles className="w-4 h-4" />
              Trusted by 50+ companies worldwide
            </motion.div>

            <motion.h1
              initial={{ opacity: 0, y: 30 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.6, delay: 0.1 }}
              className="text-4xl sm:text-5xl lg:text-6xl xl:text-7xl font-extrabold text-slate-900 dark:text-white leading-tight"
            >
              Build Digital
              <br />
              <span className="gradient-text">Products That</span>
              <br />
              <span className="text-slate-700 dark:text-slate-200">Drive Growth</span>
            </motion.h1>

            <motion.p
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.6, delay: 0.2 }}
              className="mt-6 text-lg sm:text-xl text-slate-500 dark:text-slate-400 max-w-xl mx-auto lg:mx-0 leading-relaxed"
            >
              ArinaSystems crafts high-performance web apps, mobile solutions, and custom software
              that transform your business vision into powerful digital reality.
            </motion.p>

            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.6, delay: 0.3 }}
              className="mt-8 flex flex-col sm:flex-row items-center gap-4 justify-center lg:justify-start"
            >
              <a
                href="#contact"
                className="group inline-flex items-center gap-2 px-7 py-3.5 text-base font-semibold text-white bg-gradient-to-r from-indigo-600 to-violet-600 rounded-2xl shadow-xl hover:shadow-indigo-500/40 hover:from-indigo-500 hover:to-violet-500 transition-all duration-200 active:scale-95"
              >
                Start Your Project
                <ArrowRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </a>
              <a
                href="#portfolio"
                className="group inline-flex items-center gap-2 px-7 py-3.5 text-base font-semibold text-slate-700 dark:text-slate-200 bg-slate-100 dark:bg-slate-800 rounded-2xl hover:bg-slate-200 dark:hover:bg-slate-700 transition-all duration-200 active:scale-95"
              >
                <Play className="w-4 h-4 text-indigo-600 dark:text-indigo-400" />
                View Our Work
              </a>
            </motion.div>

            {/* Stats */}
            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.6, delay: 0.4 }}
              className="mt-12 flex flex-wrap gap-8 justify-center lg:justify-start"
            >
              {[
                { value: "50+", label: "Projects Delivered" },
                { value: "98%", label: "Client Satisfaction" },
                { value: "5+", label: "Years Experience" },
              ].map((stat) => (
                <div key={stat.label} className="text-center lg:text-left">
                  <p className="text-3xl font-bold text-slate-900 dark:text-white">{stat.value}</p>
                  <p className="text-sm text-slate-500 dark:text-slate-400 mt-0.5">{stat.label}</p>
                </div>
              ))}
            </motion.div>
          </div>

          {/* Right visual */}
          <motion.div
            initial={{ opacity: 0, scale: 0.9 }}
            animate={{ opacity: 1, scale: 1 }}
            transition={{ duration: 0.8, delay: 0.2 }}
            className="flex-1 w-full max-w-lg lg:max-w-none"
          >
            <div className="relative">
              {/* Main card */}
              <div className="relative bg-gradient-to-br from-indigo-500 via-violet-600 to-cyan-500 rounded-3xl p-1 shadow-2xl">
                <div className="bg-slate-900 rounded-[22px] overflow-hidden">
                  {/* Fake browser bar */}
                  <div className="flex items-center gap-2 px-4 py-3 bg-slate-800/80 border-b border-slate-700">
                    <div className="flex gap-1.5">
                      <div className="w-3 h-3 rounded-full bg-red-400" />
                      <div className="w-3 h-3 rounded-full bg-yellow-400" />
                      <div className="w-3 h-3 rounded-full bg-green-400" />
                    </div>
                    <div className="flex-1 bg-slate-700 rounded-lg px-3 py-1 text-xs text-slate-400 text-center">
                      arinasystems.com
                    </div>
                  </div>
                  {/* Content mockup */}
                  <div className="p-6 space-y-4">
                    <div className="h-5 bg-indigo-500/30 rounded-full w-3/4 animate-pulse" />
                    <div className="h-3 bg-slate-700 rounded-full w-full" />
                    <div className="h-3 bg-slate-700 rounded-full w-5/6" />
                    <div className="grid grid-cols-2 gap-3 mt-4">
                      {[1, 2, 3, 4].map((i) => (
                        <div key={i} className="bg-slate-800 rounded-xl p-4 space-y-2">
                          <div className="w-8 h-8 bg-indigo-500/30 rounded-lg" />
                          <div className="h-2.5 bg-slate-700 rounded-full w-full" />
                          <div className="h-2 bg-slate-700/60 rounded-full w-4/5" />
                        </div>
                      ))}
                    </div>
                    <div className="h-9 bg-gradient-to-r from-indigo-600 to-violet-600 rounded-xl mt-2" />
                  </div>
                </div>
              </div>

              {/* Floating badges */}
              <motion.div
                animate={{ y: [-8, 0, -8] }}
                transition={{ duration: 3, repeat: Infinity, ease: "easeInOut" }}
                className="absolute -top-4 -left-4 bg-white dark:bg-slate-800 rounded-2xl shadow-xl px-4 py-2.5 flex items-center gap-2 border border-slate-100 dark:border-slate-700"
              >
                <div className="w-8 h-8 bg-green-100 dark:bg-green-900/50 rounded-xl flex items-center justify-center">
                  <span className="text-green-600 text-sm font-bold">✓</span>
                </div>
                <div>
                  <p className="text-xs font-semibold text-slate-700 dark:text-slate-200">Project Live</p>
                  <p className="text-[10px] text-slate-400">Deployed 2 min ago</p>
                </div>
              </motion.div>

              <motion.div
                animate={{ y: [8, 0, 8] }}
                transition={{ duration: 3.5, repeat: Infinity, ease: "easeInOut", delay: 0.5 }}
                className="absolute -bottom-4 -right-4 bg-white dark:bg-slate-800 rounded-2xl shadow-xl px-4 py-2.5 flex items-center gap-2 border border-slate-100 dark:border-slate-700"
              >
                <div className="w-8 h-8 bg-indigo-100 dark:bg-indigo-900/50 rounded-xl flex items-center justify-center">
                  <span className="text-indigo-600 text-sm">⚡</span>
                </div>
                <div>
                  <p className="text-xs font-semibold text-slate-700 dark:text-slate-200">Performance</p>
                  <p className="text-[10px] text-slate-400">Score: 98 / 100</p>
                </div>
              </motion.div>
            </div>
          </motion.div>
        </div>
      </div>
    </section>
  );
}

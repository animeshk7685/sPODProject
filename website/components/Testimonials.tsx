"use client";

import { motion } from "framer-motion";
import { Star, Quote } from "lucide-react";

const testimonials = [
  {
    name: "Sarah Mitchell",
    role: "CEO, LaunchPad Ventures",
    avatar: "SM",
    rating: 5,
    quote:
      "ArinaSystems delivered our platform on time and within budget. The team was proactive, communicative, and the quality exceeded our expectations. Highly recommend them for any complex project.",
    color: "from-indigo-500 to-violet-500",
  },
  {
    name: "James Okafor",
    role: "CTO, HealthBridge Inc.",
    avatar: "JO",
    rating: 5,
    quote:
      "Working with ArinaSystems was a game changer. They transformed our idea into a robust mobile app in just 3 months. Their attention to UX and performance is outstanding.",
    color: "from-cyan-500 to-blue-500",
  },
  {
    name: "Priya Sharma",
    role: "Product Manager, RetailFlow",
    avatar: "PS",
    rating: 5,
    quote:
      "From discovery to deployment, the ArinaSystems team was a true partner. They asked the right questions, challenged assumptions, and delivered a product our users love.",
    color: "from-pink-500 to-rose-500",
  },
];

export default function Testimonials() {
  return (
    <section id="testimonials" className="py-24 bg-slate-50 dark:bg-slate-900">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <motion.div
          initial={{ opacity: 0, y: 30 }}
          whileInView={{ opacity: 1, y: 0 }}
          viewport={{ once: true }}
          transition={{ duration: 0.6 }}
          className="text-center mb-16"
        >
          <span className="inline-block px-4 py-1.5 rounded-full bg-pink-100 dark:bg-pink-900/40 text-pink-600 dark:text-pink-400 text-sm font-semibold mb-4">
            Testimonials
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-extrabold text-slate-900 dark:text-white">
            Loved by Our
            <span className="gradient-text"> Clients</span>
          </h2>
          <p className="mt-4 text-lg text-slate-500 dark:text-slate-400 max-w-2xl mx-auto">
            Don't just take our word for it — here's what our clients have to say about working
            with ArinaSystems.
          </p>
        </motion.div>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          {testimonials.map((t, i) => (
            <motion.div
              key={t.name}
              initial={{ opacity: 0, y: 40 }}
              whileInView={{ opacity: 1, y: 0 }}
              viewport={{ once: true }}
              transition={{ duration: 0.5, delay: i * 0.12 }}
              whileHover={{ y: -4 }}
              className="bg-white dark:bg-slate-950 rounded-2xl p-6 border border-slate-200 dark:border-slate-800 hover:shadow-xl transition-all duration-300"
            >
              <Quote className="w-8 h-8 text-indigo-200 dark:text-indigo-800 mb-4" />
              <p className="text-slate-600 dark:text-slate-400 leading-relaxed text-sm mb-6">
                "{t.quote}"
              </p>
              <div className="flex items-center gap-1 mb-4">
                {Array.from({ length: t.rating }).map((_, j) => (
                  <Star key={j} className="w-4 h-4 text-yellow-400 fill-yellow-400" />
                ))}
              </div>
              <div className="flex items-center gap-3">
                <div
                  className={`w-10 h-10 rounded-xl bg-gradient-to-br ${t.color} flex items-center justify-center text-white text-sm font-bold`}
                >
                  {t.avatar}
                </div>
                <div>
                  <p className="font-semibold text-slate-900 dark:text-white text-sm">{t.name}</p>
                  <p className="text-xs text-slate-500 dark:text-slate-400">{t.role}</p>
                </div>
              </div>
            </motion.div>
          ))}
        </div>
      </div>
    </section>
  );
}

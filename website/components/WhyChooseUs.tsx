"use client";

import { motion } from "framer-motion";
import { Zap, ShieldCheck, Users, TrendingUp, Clock, HeartHandshake } from "lucide-react";

const reasons = [
  {
    icon: Zap,
    title: "Lightning-Fast Delivery",
    description:
      "We work in agile sprints to deliver working software quickly, keeping your project on track and on budget.",
    color: "text-yellow-500",
    bg: "bg-yellow-50 dark:bg-yellow-900/20",
  },
  {
    icon: ShieldCheck,
    title: "Enterprise-Grade Quality",
    description:
      "Rigorous QA processes, code reviews, and security audits ensure every line of code meets the highest standards.",
    color: "text-green-500",
    bg: "bg-green-50 dark:bg-green-900/20",
  },
  {
    icon: Users,
    title: "Dedicated Teams",
    description:
      "Your project gets a dedicated team — developer, designer, and project manager — fully aligned with your goals.",
    color: "text-blue-500",
    bg: "bg-blue-50 dark:bg-blue-900/20",
  },
  {
    icon: TrendingUp,
    title: "Scalable Architecture",
    description:
      "We build for growth. Our solutions scale seamlessly as your user base and business requirements evolve.",
    color: "text-violet-500",
    bg: "bg-violet-50 dark:bg-violet-900/20",
  },
  {
    icon: Clock,
    title: "On-Time, On-Budget",
    description:
      "Transparent timelines and milestone-based delivery mean no surprises — just consistent, reliable execution.",
    color: "text-orange-500",
    bg: "bg-orange-50 dark:bg-orange-900/20",
  },
  {
    icon: HeartHandshake,
    title: "Long-Term Partnership",
    description:
      "We invest in your success beyond launch, offering maintenance, support, and strategic product guidance.",
    color: "text-pink-500",
    bg: "bg-pink-50 dark:bg-pink-900/20",
  },
];

const container = {
  hidden: {},
  show: { transition: { staggerChildren: 0.1 } },
};

const item = {
  hidden: { opacity: 0, scale: 0.92 },
  show: { opacity: 1, scale: 1, transition: { duration: 0.45 } },
};

export default function WhyChooseUs() {
  return (
    <section id="why-us" className="py-24 bg-white dark:bg-slate-950">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <motion.div
          initial={{ opacity: 0, y: 30 }}
          whileInView={{ opacity: 1, y: 0 }}
          viewport={{ once: true }}
          transition={{ duration: 0.6 }}
          className="text-center mb-16"
        >
          <span className="inline-block px-4 py-1.5 rounded-full bg-violet-100 dark:bg-violet-900/40 text-violet-600 dark:text-violet-400 text-sm font-semibold mb-4">
            Why ArinaSystems
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-extrabold text-slate-900 dark:text-white">
            The Partner You Can
            <span className="gradient-text"> Count On</span>
          </h2>
          <p className="mt-4 text-lg text-slate-500 dark:text-slate-400 max-w-2xl mx-auto">
            We combine technical excellence with a client-first mindset to deliver outcomes that
            truly matter to your business.
          </p>
        </motion.div>

        <motion.div
          variants={container}
          initial="hidden"
          whileInView="show"
          viewport={{ once: true }}
          className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6"
        >
          {reasons.map((reason) => {
            const Icon = reason.icon;
            return (
              <motion.div
                key={reason.title}
                variants={item}
                whileHover={{ y: -4 }}
                className="flex gap-4 p-6 rounded-2xl bg-slate-50 dark:bg-slate-900 border border-slate-200 dark:border-slate-800 hover:shadow-lg transition-all duration-300"
              >
                <div
                  className={`flex-shrink-0 w-11 h-11 rounded-xl ${reason.bg} flex items-center justify-center`}
                >
                  <Icon className={`w-5 h-5 ${reason.color}`} />
                </div>
                <div>
                  <h3 className="font-bold text-slate-900 dark:text-white mb-1.5">{reason.title}</h3>
                  <p className="text-sm text-slate-500 dark:text-slate-400 leading-relaxed">
                    {reason.description}
                  </p>
                </div>
              </motion.div>
            );
          })}
        </motion.div>
      </div>
    </section>
  );
}

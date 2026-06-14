"use client";

import { motion } from "framer-motion";
import { ExternalLink } from "lucide-react";

const projects = [
  {
    title: "E-Commerce Platform",
    category: "Web Development",
    description:
      "A high-performance online store with real-time inventory, secure payments, and an intuitive admin dashboard.",
    tech: ["Next.js", "Node.js", "PostgreSQL"],
    gradient: "from-indigo-500 to-cyan-500",
    emoji: "🛒",
  },
  {
    title: "HealthTrack Mobile App",
    category: "Mobile Development",
    description:
      "Cross-platform wellness app featuring AI-powered health insights, activity tracking, and personalized coaching.",
    tech: ["React Native", "Firebase", "ML Kit"],
    gradient: "from-violet-500 to-pink-500",
    emoji: "🏃",
  },
  {
    title: "SpaceRun Arcade Game",
    category: "Game Development",
    description:
      "Addictive browser-based space arcade with leaderboards, power-ups, and stunning particle effects.",
    tech: ["Unity", "WebGL", "C#"],
    gradient: "from-amber-500 to-orange-500",
    emoji: "🚀",
  },
  {
    title: "Enterprise HR Suite",
    category: "Custom Software",
    description:
      "End-to-end HR management platform automating payroll, onboarding, and performance reviews for 500+ employees.",
    tech: ["React", "Django", "AWS"],
    gradient: "from-green-500 to-teal-500",
    emoji: "🏢",
  },
  {
    title: "Real-Estate Portal",
    category: "Web Development",
    description:
      "Property discovery platform with AI-powered recommendation engine, virtual tours, and mortgage calculators.",
    tech: ["Vue.js", "FastAPI", "Redis"],
    gradient: "from-cyan-500 to-blue-500",
    emoji: "🏠",
  },
  {
    title: "FinDash Analytics",
    category: "Custom Software",
    description:
      "Financial analytics dashboard delivering real-time market insights and portfolio tracking for investment firms.",
    tech: ["React", "Python", "Kafka"],
    gradient: "from-rose-500 to-violet-500",
    emoji: "📊",
  },
];

const container = {
  hidden: {},
  show: { transition: { staggerChildren: 0.1 } },
};

const item = {
  hidden: { opacity: 0, y: 30 },
  show: { opacity: 1, y: 0, transition: { duration: 0.5 } },
};

export default function Portfolio() {
  return (
    <section id="portfolio" className="py-24 bg-white dark:bg-slate-950">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <motion.div
          initial={{ opacity: 0, y: 30 }}
          whileInView={{ opacity: 1, y: 0 }}
          viewport={{ once: true }}
          transition={{ duration: 0.6 }}
          className="text-center mb-16"
        >
          <span className="inline-block px-4 py-1.5 rounded-full bg-cyan-100 dark:bg-cyan-900/40 text-cyan-600 dark:text-cyan-400 text-sm font-semibold mb-4">
            Our Work
          </span>
          <h2 className="text-3xl sm:text-4xl lg:text-5xl font-extrabold text-slate-900 dark:text-white">
            Projects That
            <span className="gradient-text"> Make an Impact</span>
          </h2>
          <p className="mt-4 text-lg text-slate-500 dark:text-slate-400 max-w-2xl mx-auto">
            A selection of products and platforms we've built for forward-thinking clients across
            diverse industries.
          </p>
        </motion.div>

        <motion.div
          variants={container}
          initial="hidden"
          whileInView="show"
          viewport={{ once: true }}
          className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6"
        >
          {projects.map((project) => (
            <motion.div
              key={project.title}
              variants={item}
              whileHover={{ y: -6 }}
              className="group bg-slate-50 dark:bg-slate-900 rounded-2xl overflow-hidden border border-slate-200 dark:border-slate-800 hover:shadow-xl hover:border-indigo-300 dark:hover:border-indigo-700 transition-all duration-300"
            >
              {/* Project image placeholder */}
              <div
                className={`h-44 bg-gradient-to-br ${project.gradient} flex items-center justify-center relative overflow-hidden`}
              >
                <span className="text-7xl">{project.emoji}</span>
                <div className="absolute inset-0 bg-black/10" />
                <span className="absolute top-3 left-3 px-2.5 py-1 bg-black/30 backdrop-blur-sm rounded-full text-white text-xs font-medium">
                  {project.category}
                </span>
                <div className="absolute top-3 right-3 opacity-0 group-hover:opacity-100 transition-opacity">
                  <div className="w-8 h-8 bg-white/20 backdrop-blur-sm rounded-lg flex items-center justify-center">
                    <ExternalLink className="w-4 h-4 text-white" />
                  </div>
                </div>
              </div>
              <div className="p-5">
                <h3 className="font-bold text-slate-900 dark:text-white mb-2">{project.title}</h3>
                <p className="text-sm text-slate-500 dark:text-slate-400 leading-relaxed mb-4">
                  {project.description}
                </p>
                <div className="flex flex-wrap gap-2">
                  {project.tech.map((t) => (
                    <span
                      key={t}
                      className="px-2.5 py-1 bg-indigo-50 dark:bg-indigo-900/30 text-indigo-700 dark:text-indigo-300 text-xs font-medium rounded-full border border-indigo-200 dark:border-indigo-800"
                    >
                      {t}
                    </span>
                  ))}
                </div>
              </div>
            </motion.div>
          ))}
        </motion.div>
      </div>
    </section>
  );
}

# GLIMPSE: Platform System (GLPS)

🚀 **Glimpse (GLPS)** is a cross-platform **windowing and platform system** with a current focus on **Wayland backend development**. Designed for flexibility, performance, and developer sanity (we try, at least).

---

## 🌍 Why GLPS?
Ever wanted a **lightweight, modern** alternative to GLFW that doesn't make you wrestle with legacy code? GLPS is here to give you **low-level control** without sacrificing usability. Whether you're managing windows or handling input devices, **GLPS has your back**.

---

## 📌 Current Status

![Wayland Support](preview.gif)

GLPS is actively growing! Here's a look at what’s already in place and what’s brewing:

### ✅ Feature Matrix

| Category            | Feature               | Status      | Details |
|--------------------|----------------------|------------|---------|
| **Platform Support** | Wayland Backend       | ✅ Implemented  | Core Wayland protocol functionality |
|                    | Multi-Platform Planning | ⬜ Roadmap   | Future cross-platform expansion |
| **Window Management** | Multi-Window Support  | ✅ Implemented  | Unique contexts per window |
|                    | High DPI Scaling       | ⬜ Planned    | Resolution adaptive interfaces |
| **Graphics Integration** | EGL Support       | ✅ Implemented  | Display, context, surface management |
|                    | OpenGL Contexts      | ✅ Implemented  | GLPS handles context creation, you do the rest |
|                    | Vulkan Support       | ⬜ Planned    | Additional backend support |
| **Input Handling**  | Keyboard Input       | ✅ Implemented  | Wayland keyboard events |
|                    | Mouse Input          | ✅ Implemented  | Pointer event management |
|                    | Touchscreen Support  | ✅ Implemented  | Touch input events |
| **Advanced Features** | Wayland Compositor  | ✅ Implemented  | Registry interactions |
|                    | XDG-Shell Support    | ✅ Implemented  | Surface and toplevel management |
|                    | Clipboard Integration | ✅ Implemented  | Cross-application data transfer |
|                    | Drag-and-Drop        | ✅ Implemented  | Enhanced user interaction |
| **Development Tools** | Logging            | ✅ Implemented  | [Pico Logger](https://github.com/YASSINE-AA/Pico-Logger) integration |

---

## 🔮 Upcoming Development Priorities
1️⃣ **Multi-Monitor Support** – Because one screen is never enough.
2️⃣ **Cross-Platform Compatibility** – Bringing the magic to more platforms.
3️⃣ **Expanded Input Method Support** – Because not everyone uses a keyboard and mouse.

---

## 🛠️ Logging & Debugging

GLPS is **fully integrated** with [Pico Logger](https://github.com/YASSINE-AA/Pico-Logger), ensuring **structured, real-time** debugging without the chaos of scattered `printf`s.

---

## 🤝 Contributing

Got ideas? Bugs? An existential crisis about windowing systems? **Join the GLPS project!** Check out our contribution guidelines and roadmap for ways to help.

---

## 📜 License

GLPS is licensed under the **MIT License**, meaning you’re free to use, modify, and distribute it—just don’t blame us if your GPU catches fire. 🔥

---

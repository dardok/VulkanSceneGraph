﻿#pragma once

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <vsg/platform/wayland/wayland-xdg-shell-client-protocol.h>
#include <vsg/platform/wayland/xdg-decoration-client.h>
#include <vsg/ui/PointerEvent.h>
#include <vulkan/vulkan_wayland.h>

namespace vsgWayland
{

    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using KeycodeModifier = std::pair<uint32_t, uint32_t>;
        using KeycodeMap = std::map<KeycodeModifier, vsg::KeySymbol>;

        void add(uint32_t keycode, uint32_t modifier, vsg::KeySymbol key);

        void add(uint32_t keycode, std::initializer_list<std::pair<uint32_t, vsg::KeySymbol>> combinations);

        vsg::KeySymbol getKeySymbol(uint32_t keycode, uint32_t modifier);
        vsg::KeyModifier getKeyModifier(vsg::KeySymbol keySym, uint32_t modifier, bool pressed);

    protected:
        KeycodeMap _keycodeMap;
        uint32_t _modifierMask;
    };

    class Wayland_surface : public vsg::Surface
    {
    public:
        Wayland_surface(vsg::Instance* instance, wl_display* wlDisplay, wl_surface* wlSurface);
    };

    class Wayland_Window : public vsg::Inherit<vsg::Window, Wayland_Window>
    {
    public:
        Wayland_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Wayland_Window() = delete;
        Wayland_Window(const Wayland_Window&) = delete;
        Wayland_Window& operator=(const Wayland_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME; }

        bool valid() const override;

        bool visible() const override;

        void releaseWindow() override;
        void releaseConnection() override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;
    protected:
        ~Wayland_Window();

        void _initSurface() override;

        static void keymapEvent(void* data, wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
        static void kbd_enter_event(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
        static void kbd_leave_event(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface);
        static void kbd_key_event(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
        static void kbd_modifier_event(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
        constexpr static struct wl_keyboard_listener wl_keyboard_listener = {
            .keymap = keymapEvent,
            .enter = kbd_enter_event,
            .leave = kbd_leave_event,
            .key = kbd_key_event,
            .modifiers = kbd_modifier_event
        };

        static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial);
        constexpr static struct xdg_surface_listener xdg_surface_listener = {
            .configure = xdg_surface_handle_configure,
        };

        static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states);
        static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel);
        constexpr static struct xdg_toplevel_listener xdg_toplevel_listener = {
            .configure = xdg_toplevel_handle_configure,
            .close = xdg_toplevel_handle_close,
        };

        static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);
        constexpr static struct xdg_wm_base_listener xdg_wm_base_listener = {
            .ping = xdg_wm_base_ping
        };

        static void pointer_enter (void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void pointer_leave (void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface);
        static void pointer_motion (void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
        static void pointer_button (void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        static void pointer_axis (void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
        constexpr static struct wl_pointer_listener pointer_listener = {
            &pointer_enter,
            &pointer_leave,
            &pointer_motion,
            &pointer_button,
            &pointer_axis
        };

        static void seat_capabilities (void *data, struct wl_seat *seat, uint32_t capabilities);
        constexpr static struct wl_seat_listener seat_listener = {
            &seat_capabilities
        };

        static void registry_add_object(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
        static void registry_remove_object(void *data, struct wl_registry *registry, uint32_t id);
        constexpr static struct wl_registry_listener registry_listener = {
            &registry_add_object,
            &registry_remove_object
        };

        static void shell_surface_ping (void *data, struct wl_shell_surface *shell_surface, uint32_t serial);
        static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height);
        static void shell_surface_popup_done (void *data, struct wl_shell_surface *shell_surface);
        constexpr static struct wl_shell_surface_listener shell_surface_listener = {
            &shell_surface_ping,
            &shell_surface_configure,
            &shell_surface_popup_done
        };

        struct wl_display* _wlDisplay = nullptr;
        struct wl_registry* _wlRegistry = nullptr;
        struct wl_compositor* _wlCompositor = nullptr;
        struct wl_seat* _seat = nullptr;
        struct wl_shm* _shm = nullptr;
        struct wl_cursor_theme* _cursorTheme = nullptr;
        struct wl_surface* _cursorSurface = nullptr;
        struct zxdg_decoration_manager_v1* _decorationManager = nullptr;
        struct xdg_wm_base* _xdgWmBase = nullptr;
        //struct wl_subcompositor *subcompositor;
        struct wl_surface* _wlSurface = nullptr;
        struct xdg_surface* _xdgSurface = nullptr;
        struct xdg_toplevel* _xdgToplevel = nullptr;
        struct wl_surface* _currentSurface = nullptr;
        struct zxdg_toplevel_decoration_v1* _topDecoration = nullptr;
        struct xkb_state *_xkb_state;
        struct xkb_context *_xkb_context;
        struct xkb_keymap *_xkb_keymap;

        int _width = 256;
        int _height = 256;
        int cursor_x = -1;
        int cursor_y = -1;
        bool _resize = false;
        uint16_t maskButtons;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };
} // namespace vsgWayland

EVSG_type_name(vsgWayland::Wayland_Window);

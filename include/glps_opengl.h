/**
 * @file glps_opengl.h
 * @brief Header file for OpenGL rendering functions in the GLPS library.
 */

#ifndef GLPS_OPENGL_H
#define GLPS_OPENGL_H

#include "glps_common.h"

/**
 * @brief Initializes OpenGL for the given window manager.
 * @param wm Pointer to the GLPS window manager.
 */
void glps_opengl_init(glps_WindowManager *wm);

/**
 * @brief Sets up shared OpenGL resources for the given window manager.
 * @param wm Pointer to the GLPS window manager.
 */
void glps_opengl_setup_shared(glps_WindowManager *wm);

/**
 * @brief Initializes FreeType for text rendering.
 * @param wm Pointer to the GLPS window manager.
 * @param font_path Path to the font file.
 * @return 0 on success, non-zero on failure.
 */
int glps_ft_init(glps_WindowManager *wm, const char *font_path);

/**
 * @brief Sets up OpenGL resources specific to a window.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to set up.
 */
void glps_opengl_setup_separate(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Cleans up OpenGL resources for the given window manager.
 * @param wm Pointer to the GLPS window manager.
 */
void glps_opengl_cleanup(glps_WindowManager *wm);

/**
 * @brief Clears the OpenGL buffer for rendering.
 * @param wm Pointer to the GLPS window manager.
 */
void glps_clear(glps_WindowManager *wm);

/**
 * @brief Sets the text projection matrix for a specific window.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to set the projection matrix for.
 */
void glps_opengl_set_text_projection(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Draws window borders with the specified color.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to draw borders for.
 * @param color Border color in ARGB format.
 */
void glps_opengl_draw_window_borders(glps_WindowManager *wm, size_t window_id, unsigned int color);

/**
 * @brief Draws a rectangle outline.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to draw in.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @param color Rectangle color in ARGB format.
 */
void glps_opengl_draw_rectangle(glps_WindowManager *wm, size_t window_id, int x, int y, int width, int height, unsigned int color);

/**
 * @brief Draws a filled rectangle.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to draw in.
 * @param x X-coordinate of the rectangle's top-left corner.
 * @param y Y-coordinate of the rectangle's top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @param color Rectangle color in ARGB format.
 */
void glps_opengl_fill_rectangle(glps_WindowManager *wm, size_t window_id, int x, int y, int width, int height, unsigned int color);

/**
 * @brief Draws a filled ellipse or circle.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to draw in.
 * @param x_center X-coordinate of the ellipse's center.
 * @param y_center Y-coordinate of the ellipse's center.
 * @param width Width of the ellipse.
 * @param height Height of the ellipse.
 * @param color Ellipse color in ARGB format.
 */
void glps_opengl_fill_circle(glps_WindowManager *wm, size_t window_id, int x_center, int y_center, int width, int height, unsigned int color);

/**
 * @brief Draws a straight line between two points.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to draw in.
 * @param x1 X-coordinate of the starting point.
 * @param y1 Y-coordinate of the starting point.
 * @param x2 X-coordinate of the ending point.
 * @param y2 Y-coordinate of the ending point.
 * @param color Line color in ARGB format.
 */
void glps_opengl_draw_line(glps_WindowManager *wm, size_t window_id, int x1, int y1, int x2, int y2, unsigned int color);

/**
 * @brief Renders text on the window using FreeType.
 * @param wm Pointer to the GLPS window manager.
 * @param window_id ID of the window to render text in.
 * @param x X-coordinate for the text's starting position.
 * @param y Y-coordinate for the text's starting position.
 * @param text Pointer to the text string to render.
 * @param font_size Font size for the text.
 * @param color Text color in ARGB format.
 */
void glps_opengl_draw_text(glps_WindowManager *wm, size_t window_id, int x, int y, const char *text, float font_size, unsigned int color);

#endif // GLPS_OPENGL_H

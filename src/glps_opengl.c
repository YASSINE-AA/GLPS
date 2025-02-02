#include "glps_opengl.h"

static void check_shader_compile(GLuint shader) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    // LOG_ERROR("ERROR: Shader compilation failed\n%s\n", infoLog);
    exit(EXIT_FAILURE);
  }
}

static void check_shader_link(GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    //  LOG_ERROR("ERROR: Program linking failed\n%s\n", infoLog);
    exit(EXIT_FAILURE);
  }
}

void glps_opengl_init(glps_WindowManager *wm) {
  if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }
}

void glps_opengl_setup_shared(glps_WindowManager *wm) {
  wm->shared_ogl_ctx = malloc(sizeof(glps_SharedOpenGLContext));
  glGenBuffers(1, &wm->shared_ogl_ctx->text_vbo);
  wm->shared_ogl_ctx->text_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  set_shader_src_file("shaders/text/text_vertex.glsl",
                      wm->shared_ogl_ctx->text_vertex_shader);

  glCompileShader(wm->shared_ogl_ctx->text_vertex_shader);
  check_shader_compile(wm->shared_ogl_ctx->text_vertex_shader);

  wm->shared_ogl_ctx->text_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  set_shader_src_file("shaders/text/text_fragment.glsl",
                      wm->shared_ogl_ctx->text_fragment_shader);
  glCompileShader(wm->shared_ogl_ctx->text_fragment_shader);
  check_shader_compile(wm->shared_ogl_ctx->text_fragment_shader);

  glGenBuffers(1, &wm->shared_ogl_ctx->shape_vbo);

  GLuint shape_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  set_shader_src_file("shaders/shape/shape_vertex.glsl", shape_vertex_shader);
  glCompileShader(shape_vertex_shader);
  check_shader_compile(shape_vertex_shader);

  GLuint shape_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  set_shader_src_file("shaders/shape/shape_fragment.glsl",
                      shape_fragment_shader);
  glCompileShader(shape_fragment_shader);
  check_shader_compile(shape_fragment_shader);

  wm->shared_ogl_ctx->shape_program = glCreateProgram();
  glAttachShader(wm->shared_ogl_ctx->shape_program, shape_vertex_shader);
  glAttachShader(wm->shared_ogl_ctx->shape_program, shape_fragment_shader);
  glLinkProgram(wm->shared_ogl_ctx->shape_program);
  check_shader_link(wm->shared_ogl_ctx->shape_program);

  glDeleteShader(shape_vertex_shader);
  glDeleteShader(shape_fragment_shader);
}

void glps_opengl_setup_separate(glps_WindowManager *wm, size_t window_id) {

  wm->windows[window_id]->specific_ogl_ctx->text_program = glCreateProgram();
  glAttachShader(wm->windows[window_id]->specific_ogl_ctx->text_program,
                 wm->shared_ogl_ctx->text_vertex_shader);
  glAttachShader(wm->windows[window_id]->specific_ogl_ctx->text_program,
                 wm->shared_ogl_ctx->text_fragment_shader);
  glLinkProgram(wm->windows[window_id]->specific_ogl_ctx->text_program);
  check_shader_link(wm->windows[window_id]->specific_ogl_ctx->text_program);

  GLuint text_vao;
  glGenVertexArrays(1, &text_vao);

  glBindVertexArray(text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->text_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  wm->windows[window_id]->specific_ogl_ctx->text_vao = text_vao;
  glBindVertexArray(0);

  GLuint shape_vao;
  glGenVertexArrays(1, &shape_vao);
  glBindVertexArray(shape_vao);
  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->shape_vbo);

  GLint position_attrib =
      glGetAttribLocation(wm->shared_ogl_ctx->shape_program, "pos");
  glEnableVertexAttribArray(position_attrib);
  glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, pos));

  GLint col_attrib =
      glGetAttribLocation(wm->shared_ogl_ctx->shape_program, "col");
  glEnableVertexAttribArray(col_attrib);
  glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, col));
  wm->windows[window_id]->specific_ogl_ctx->shape_vao = shape_vao;
}

void glps_opengl_cleanup(glps_WindowManager *wm) {
  if (wm->shared_ogl_ctx) {
    if (wm->shared_ogl_ctx->text_vbo) {
      glDeleteBuffers(1, &wm->shared_ogl_ctx->text_vbo);
      wm->shared_ogl_ctx->text_vbo = 0;
    }
    if (wm->shared_ogl_ctx->shape_vbo) {
      glDeleteBuffers(1, &wm->shared_ogl_ctx->shape_vbo);
      wm->shared_ogl_ctx->shape_vbo = 0;
    }
    if (wm->shared_ogl_ctx->shape_program) {
      glDeleteProgram(wm->shared_ogl_ctx->shape_program);
      wm->shared_ogl_ctx->shape_program = 0;
    }
    free(wm->shared_ogl_ctx);
    wm->shared_ogl_ctx = NULL;
  }

  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i]->specific_ogl_ctx) {
      if (wm->windows[i]->specific_ogl_ctx->text_vao) {
        glDeleteVertexArrays(1, &wm->windows[i]->specific_ogl_ctx->text_vao);
        wm->windows[i]->specific_ogl_ctx->text_vao = 0;
      }
      if (wm->windows[i]->specific_ogl_ctx->shape_vao) {
        glDeleteVertexArrays(1, &wm->windows[i]->specific_ogl_ctx->shape_vao);
        wm->windows[i]->specific_ogl_ctx->shape_vao = 0;
      }
      if (wm->windows[i]->specific_ogl_ctx->text_program) {
        glDeleteProgram(wm->windows[i]->specific_ogl_ctx->text_program);
        wm->windows[i]->specific_ogl_ctx->text_program = 0;
      }
      free(wm->windows[i]->specific_ogl_ctx);
      wm->windows[i]->specific_ogl_ctx = NULL;
    }
  }
}

void convert_coords_to_ndc(float *ndc_x, float *ndc_y, int x, int y,
                           int screen_width, int screen_height) {
  *ndc_x = (2.0f * x) / screen_width - 1.0f;
  *ndc_y = 1.0f - (2.0f * y) / screen_height;
}

void convert_dimension_to_ndc(float *ndc_width, float *ndc_height, int width,
                              int height, int screen_width, int screen_height) {
  *ndc_width = (2.0f * width) / screen_width;
  *ndc_height = (2.0f * height) / screen_height;
}

void convert_hex_to_rgb(vec3 *rgb, unsigned int color_hex) {
  (*rgb)[0] = ((color_hex >> 16) & 0xFF) / 255.0f;
  (*rgb)[1] = ((color_hex >> 8) & 0xFF) / 255.0f;
  (*rgb)[2] = ((color_hex) & 0xFF) / 255.0f;
}
void glps_opengl_draw_rectangle(glps_WindowManager *wm, size_t window_id, int x,
                                int y, int width, int height,
                                unsigned int color) {

  float ndc_x, ndc_y;
  float ndc_width, ndc_height;
  vec3 color_rgb;

  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  convert_coords_to_ndc(&ndc_x, &ndc_y, x, y, window_width, window_height);
  convert_dimension_to_ndc(&ndc_width, &ndc_height, width, height, window_width,
                           window_height);
  convert_hex_to_rgb(&color_rgb, color);

  Vertex vertices[4];

  for (int i = 0; i < 4; i++) {
    vertices[i].col[0] = color_rgb[0];
    vertices[i].col[1] = color_rgb[1];
    vertices[i].col[2] = color_rgb[2];
  }

  vertices[0].pos[0] = ndc_x;
  vertices[0].pos[1] = ndc_y;
  vertices[1].pos[0] = ndc_x + ndc_width;
  vertices[1].pos[1] = ndc_y;
  vertices[2].pos[0] = ndc_x + ndc_width;
  vertices[2].pos[1] = ndc_y + ndc_height;

  vertices[3].pos[0] = ndc_x;
  vertices[3].pos[1] = ndc_y + ndc_height;
  glUseProgram(wm->shared_ogl_ctx->shape_program);

  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->shape_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->shape_vao);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void glps_opengl_fill_rectangle(glps_WindowManager *wm, size_t window_id, int x,
                                int y, int width, int height,
                                unsigned int color) {

  float ndc_x, ndc_y;
  float ndc_width, ndc_height;
  vec3 color_rgb;

  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  convert_coords_to_ndc(&ndc_x, &ndc_y, x, y, window_width, window_height);
  convert_dimension_to_ndc(&ndc_width, &ndc_height, width, height, window_width,
                           window_height);
  convert_hex_to_rgb(&color_rgb, color);

  Vertex vertices[6];

  for (int i = 0; i < 6; i++) {
    vertices[i].col[0] = color_rgb[0];
    vertices[i].col[1] = color_rgb[1];
    vertices[i].col[2] = color_rgb[2];
  }

  vertices[0].pos[0] = ndc_x;
  vertices[0].pos[1] = ndc_y;
  vertices[1].pos[0] = ndc_x + ndc_width;
  vertices[1].pos[1] = ndc_y;
  vertices[2].pos[0] = ndc_x;
  vertices[2].pos[1] = ndc_y + ndc_height;

  vertices[3].pos[0] = ndc_x + ndc_width;
  vertices[3].pos[1] = ndc_y;
  vertices[4].pos[0] = ndc_x + ndc_width;
  vertices[4].pos[1] = ndc_y + ndc_height;
  vertices[5].pos[0] = ndc_x;
  vertices[5].pos[1] = ndc_y + ndc_height;

  glUseProgram(wm->shared_ogl_ctx->shape_program);

  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->shape_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->shape_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void glps_opengl_draw_line(glps_WindowManager *wm, size_t window_id, int x1,
                           int y1, int x2, int y2, unsigned int color) {
  float ndc_x1, ndc_y1;
  float ndc_x2, ndc_y2;
  vec3 color_rgb;

  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  convert_coords_to_ndc(&ndc_x1, &ndc_y1, x1, y1, window_width, window_height);
  convert_coords_to_ndc(&ndc_x2, &ndc_y2, x2, y2, window_width, window_height);

  convert_hex_to_rgb(&color_rgb, color);

  Vertex vertices[2];

  for (int i = 0; i < 2; i++) {
    vertices[i].col[0] = color_rgb[0];
    vertices[i].col[1] = color_rgb[1];
    vertices[i].col[2] = color_rgb[2];
  }

  vertices[0].pos[0] = ndc_x1;
  vertices[0].pos[1] = ndc_y1;
  vertices[1].pos[0] = ndc_x2;
  vertices[1].pos[1] = ndc_y2;
  glUseProgram(wm->shared_ogl_ctx->shape_program);

  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->shape_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->shape_vao);
  glDrawArrays(GL_LINES, 0, 2);
}

void glps_opengl_fill_circle(glps_WindowManager *wm, size_t window_id,
                             int x_center, int y_center, int width, int height,
                             unsigned int color) {
  const int segments = 10;

  float ndc_x_center, ndc_y_center;

  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  convert_coords_to_ndc(&ndc_x_center, &ndc_y_center, x_center, y_center,
                        window_width, window_height);

  vec3 color_rgb;
  convert_hex_to_rgb(&color_rgb, color);

  Vertex vertices[segments + 2];

  vertices[0].pos[0] = ndc_x_center;
  vertices[0].pos[1] = ndc_y_center;
  vertices[0].col[0] = color_rgb[0];
  vertices[0].col[1] = color_rgb[1];
  vertices[0].col[2] = color_rgb[2];

  for (int i = 0; i <= segments; ++i) {
    float angle = (float)i / segments * 2.0f * M_PI;
    float x = x_center + (width * 0.5f * cosf(angle));
    float y = y_center + (height * 0.5f * sinf(angle));

    float ndc_x, ndc_y;
    convert_coords_to_ndc(&ndc_x, &ndc_y, x, y, window_width, window_height);

    vertices[i + 1].pos[0] = ndc_x;
    vertices[i + 1].pos[1] = ndc_y;
    vertices[i + 1].col[0] = color_rgb[0];
    vertices[i + 1].col[1] = color_rgb[1];
    vertices[i + 1].col[2] = color_rgb[2];
  }

  glUseProgram(wm->shared_ogl_ctx->shape_program);
  glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->shape_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->shape_vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
}

int glps_ft_init(glps_WindowManager *wm, const char *font_path) {

  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    LOG_ERROR("Could not initialize FreeType library\n");
    return -1;
  }

  FT_Face face;

  if (FT_New_Face(ft, font_path, 0, &face)) {
    LOG_ERROR("Failed to load font: %s\n", "Roboto");
    FT_Done_FreeType(ft);
    return -1;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      LOG_ERROR("Failed to load Glyph for character: '%c'\n", c);
      continue;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, face->glyph->bitmap.width,
                 face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character = {texture,
                           face->glyph->bitmap.width,
                           face->glyph->bitmap.rows,
                           face->glyph->bitmap_left,
                           face->glyph->bitmap_top,
                           (int)face->glyph->advance.x};
    wm->shared_ogl_ctx->characters[c] = character;
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  return 0;
}

void glps_opengl_set_text_projection(glps_WindowManager *wm, size_t window_id) {

  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  mat4x4 projection;
  mat4x4_ortho(projection, 0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);
  glUseProgram(wm->windows[window_id]->specific_ogl_ctx->text_program);
  glUniformMatrix4fv(
      glGetUniformLocation(
          wm->windows[window_id]->specific_ogl_ctx->text_program, "projection"),
      1, GL_FALSE, (const GLfloat *)projection);
  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->text_vao);
}

void glps_opengl_draw_text(glps_WindowManager *wm, size_t window_id, int x,
                           int y, const char *text, float font_size,
                           unsigned int color) {

  vec3 color_rgb;
  float ndc_x, ndc_y;
  int window_width = wm->windows[window_id]->properties.width,
      window_height = wm->windows[window_id]->properties.height;

  int initial_x = x;

  convert_coords_to_ndc(&ndc_x, &ndc_y, x, y, window_width, window_height);
  convert_hex_to_rgb(&color_rgb, color);
  glUseProgram(wm->windows[window_id]->specific_ogl_ctx->text_program);
  glUniform3f(
      glGetUniformLocation(
          wm->windows[window_id]->specific_ogl_ctx->text_program, "textColor"),
      color_rgb[0], color_rgb[1], color_rgb[2]);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(wm->windows[window_id]->specific_ogl_ctx->text_vao);

  int line_count = 0;
  int last_space_pos = -1;
  int split_pos[128];
  int line_height = 20;
  float current_x = x;

  for (size_t i = 0; i < strlen(text); i++) {
    Character ch = wm->shared_ogl_ctx->characters[(unsigned char)text[i]];
    if (ch.textureID == 0)
      continue;

    float xpos = current_x + ch.bearingX * font_size;

    if (text[i] == ' ') {
      last_space_pos = current_x;
    }

    if (xpos + ch.width * font_size > window_width) {
      if (last_space_pos != -1) {
        split_pos[line_count] = last_space_pos;
        line_count++;
      }
      current_x = initial_x;
      y += (ch.height + ch.bearingY) * font_size;
    } else {
      current_x += (ch.advance >> 6) * font_size;
    }
  }

  int current_rendering_line = 0;
  current_x = initial_x;

  for (size_t i = 0; i < strlen(text); i++) {
    Character ch = wm->shared_ogl_ctx->characters[(unsigned char)text[i]];
    if (ch.textureID == 0)
      continue;

    float xpos = current_x + ch.bearingX * font_size;
    float ypos = y - ch.bearingY * font_size;
    float w = ch.width * font_size;
    float h = ch.height * font_size;

    if (current_rendering_line < line_count &&
        xpos >= split_pos[current_rendering_line]) {
      current_x = initial_x;
      y += (ch.height + ch.bearingY) * font_size + line_height;
      xpos = current_x + ch.bearingX * font_size;
      ypos = y - ch.bearingY * font_size;
      current_rendering_line++;
    }

    if (y + h > window_height) {
      break;
    }

    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos, ypos + h, 0.0f, 0.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

    glBindTexture(GL_TEXTURE_2D, ch.textureID);
    glBindBuffer(GL_ARRAY_BUFFER, wm->shared_ogl_ctx->text_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    current_x += (ch.advance >> 6) * font_size;
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void glps_opengl_draw_window_borders(glps_WindowManager *wm, size_t window_id,
                                     unsigned int color) {
  int window_width = wm->windows[window_id]->properties.width;
  int window_height = wm->windows[window_id]->properties.height;

  // Top border
  glps_opengl_fill_rectangle(wm, window_id, 0, 40, window_width, 40, color);

  // Left border
  glps_opengl_fill_rectangle(wm, window_id, 0, window_height, 5, window_height,
                             color);

  // Right border
  glps_opengl_fill_rectangle(wm, window_id, window_width - 5, window_height, 5,
                             window_height, color);
  // Draw Window Title
  glps_opengl_draw_text(wm, window_id, 20, 25,
                        wm->windows[window_id]->properties.title, 0.3f,
                        0x000000);
  // Draw Window Buttons
  glps_opengl_fill_rectangle(wm, window_id, window_width - 40, 30, 20, 20,
                             0xFF0000);

  // Draw X close
  glps_opengl_draw_text(wm, window_id, window_width - 34, 25, "X", 0.3f,
                        0x000000);
}

void glps_clear(glps_WindowManager *wm) {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

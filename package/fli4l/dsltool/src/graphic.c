/*!
 * @file    graphic.c
 * @brief   implementaion file for graphic generation
 * @details generate png graphics using cairo and pango library
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    20.02.2013
 *
 * @note
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * @n
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * @n
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pango/pangocairo.h>

#include "graphic.h"
#include "log.h"

/*!
 * @var         static cairo_surface_t* surface
 * @brief       pointer to cairo surface
 */
static cairo_surface_t* surface = NULL;
/*!
 * @var         static cairo_t* cr
 * @brief       pointer to cairo data
 */
static cairo_t* cr = NULL;
/*!
 * @var         static PangoFontDescription* font_descr
 * @brief       pointer to pango font description
 */

static PangoFontDescription* font_descr = NULL;

/*!
 * @fn          int graphic_create(double width, double height)
 * @brief       create graphic
 * @details     create empty graphic
 *              may be called only once!
 * @param[in]   width
 *              graphic width in pixel
 * @param[out]  height
 *              graphic height in pixel
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
int graphic_create(double width, double height)
{
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (NULL == surface)
    {
        log_error("failed to create cairo surface");
        goto error_0;
    }

    cr = cairo_create(surface);
    if (NULL == cr)
    {
        log_error("failed to create cairo");
        goto error_1;
    }
    font_descr = pango_font_description_new();

    if (NULL == font_descr)
    {
        log_error("failed to create pango font");
        goto error_2;
    }

    pango_font_description_set_family(font_descr, "Courier");
    pango_font_description_set_weight(font_descr, PANGO_WEIGHT_NORMAL);
    pango_font_description_set_size(font_descr, 8 * PANGO_SCALE);

    return 0; /* success */

error_2:
    cairo_destroy(cr);
    cr = NULL;

error_1:
    cairo_surface_destroy(surface);
    surface = NULL;

error_0:
    return -1;
}

/*!
 * @fn          void graphic_delete()
 * @brief       delete graphic
 */
void graphic_delete()
{
    if (NULL != font_descr)
    {
        pango_font_description_free(font_descr);
        font_descr = NULL;
    }

    if (NULL != cr)
    {
        cairo_destroy(cr);
        cr = NULL;
    }

    if (NULL != surface)
    {
        cairo_surface_destroy(surface);
        surface = NULL;
    }
}

/*!
 * @fn          int graphic_save(const char* file)
 * @brief       save graphic
 * @details     save graphic to .PNG file
 * @param[in]   file
 *              name for .png file
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
int graphic_save(const char* file)
{
    if (NULL == surface)
    {
        return -1;
    }
    cairo_surface_write_to_png(surface, file);

    return 0;
}

/*!
 * @fn          static void graphic_color(unsigned long color,
 *                  double* r, double* g, double* b)
 * @brief       convert color
 * @param[in]   color
 *              24bit color value
 * @param[out]  r
 *              pointer to red value
 * @param[out]  g
 *              pointer to green value
 * @param[out]  b
 *              pointer to blue value
 */
static void graphic_color(unsigned long color, double* r, double* g, double* b)
{
    *r = (double) ((color >> 16) & 0x000000ff) / 255.0;
    *g = (double) ((color >> 8) & 0x000000ff) / 255.0;
    *b = (double) (color & 0x000000ff) / 255.0;
}

/*!
 * @fn          static void graphic_fit(double* x, double* y, int w)
 * @brief       fit point
 * @param[in,out] x
 *               x value
 * @param[in,out] y
 *              y valued
 * @param[in]   w
 *              width
 */
static void graphic_fit(double* x, double* y, int w)
{
    double width = w;

    if (NULL == cr)
    {
        return;
    }
    width = width / 2.0 - ceil(width / 2.0);

    cairo_user_to_device(cr, x, y);
    *x = floor(*x - 0.5) - width;
    *y = ceil(*y + 0.5) + width;
    cairo_device_to_user(cr, x, y);
}

/*!
 * @fn          int graphic_line(double x0, double y0, double x1, double y1,
 *                  double width, int dash, unsigned long color)
 * @brief       draw line
 * @param[in]   x0
 *              start point (x)
 * @param[in]   y0
 *              start point (y)
 * @param[in]   x1
 *              end point (x)
 * @param[in]   y1
 *              end point (y)
 * @param[in]   width
 *              line width
 * @param[in]   dash
 *              dash flag
 * @param[in]   color
 *              24bit color value
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
int graphic_line(double x0, double y0, double x1, double y1, double width,
        int dash, unsigned long color)
{
    double r, g, b;
    double dashes[2] =
    { 1, 1 };

    if (NULL == cr)
    {
        return -1;
    }
    graphic_color(color, &r, &g, &b);

    graphic_fit(&x0, &y0, width);
    graphic_fit(&x1, &y1, width);

    cairo_save(cr);
    cairo_new_path(cr);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, width);

    if (dash)
        cairo_set_dash(cr, dashes, 2, 0);

    cairo_move_to(cr, x0, y0);
    cairo_line_to(cr, x1, y1);
    cairo_stroke(cr);
    cairo_restore(cr);

    return 0;
}

/*!
 * @fn          int graphic_rect(double x0, double y0, double x1, double y1,
 *                  unsigned long color)
 * @brief       draw filled rectangle
 * @param[in]   x0
 *              start point (x)
 * @param[in]   y0
 *              start point (y)
 * @param[in]   x1
 *              end point (x)
 * @param[in]   y1
 *              end point (y)
 * @param[in]   color
 *              24bit color value
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
int graphic_rect(double x0, double y0, double x1, double y1,
        unsigned long color)
{
    double r, g, b;
    if (NULL == cr)
    {
        return -1;
    }
    graphic_color(color, &r, &g, &b);

    cairo_new_path(cr);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_rectangle(cr, x0, y0, x1 - x0, y1 - y0);
    cairo_fill(cr);
    cairo_close_path(cr);

    return 0;
}

/*!
 * @fn          int graphic_block(double x, double y, unsigned long color,
 *                  unsigned long color_text)
 * @brief       draw rectangle filled with different color
 * @param[in]   x
 *              upper left point (x)
 * @param[in]   y
 *              upper left point (y)
 * @param[in]   color
 *              24bit fill color value
 * @param[in]   color_text
 *              24bit rectangle color value
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
int graphic_block(double x, double y, unsigned long color,
        unsigned long color_text)
{
    double r, g, b;
    double X0 = x;
    double Y0 = y;
    double X1 = x + 8;
    double Y1 = y + 8;
    if (NULL == cr)
    {
        return -1;
    }
    graphic_color(color, &r, &g, &b);

    graphic_fit(&X0, &Y0, 1);
    graphic_fit(&X1, &Y1, 1);

    cairo_save(cr);
    cairo_new_path(cr);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_rectangle(cr, X0, Y0, X1 - X0, Y1 - Y0);
    cairo_fill(cr);
    cairo_close_path(cr);
    cairo_stroke(cr);

    cairo_new_path(cr);
    graphic_color(color_text, &r, &g, &b);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, 1);

    cairo_move_to(cr, X0, Y0);
    cairo_line_to(cr, X1, Y0);
    cairo_line_to(cr, X1, Y1);
    cairo_line_to(cr, X0, Y1);
    cairo_line_to(cr, X0, Y0);
    cairo_stroke(cr);
    cairo_restore(cr);

    return 0;
}

/*!
 * @fn          int graphic_text(double x, double y, const char* text,
 *                  int halign, int valign, int rotate, unsigned long color)
 * @brief       draw text
 * @param[in]   x
 *              start point (x)
 * @param[in]   y
 *              start left point (y)
 * @param[in]   text
 *              pointer to text
 * @param[in]   halign
 *              horizontal alignment
 * @param[in]   valign
 *              vertical alignment
 * @param[in]   rotate
 *              text rotation
 * @param[in]   color
 *              24bit color value
 * @return  error code
 * @retval  0
 *          success
 * @retval  -1
 *          failure
 */
int graphic_text(double x, double y, const char* text, int halign, int valign,
        int rotate, unsigned long color)
{
    PangoLayout* layout = NULL;
    PangoRectangle rect;
    double r, g, b;
    double sx = 0;
    double sy = 0;

    graphic_color(color, &r, &g, &b);

    if (NULL == cr)
    {
        return -1;
    }

    layout = pango_cairo_create_layout(cr);
    if (NULL == layout)
    {
        return -1;
    }

    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_set_source_rgb(cr, r, g, b);

    pango_layout_set_font_description(layout, font_descr);
    pango_layout_set_text(layout, text, -1);
    pango_layout_get_pixel_extents(layout, NULL, &rect);
    pango_cairo_update_layout(cr, layout);

    if (rotate)
    {
        cairo_rotate(cr, rotate * G_PI / 2.0);
    }

    switch (halign)
    {
    case ALIGN_CENTER:
        sx -= rect.width / 2;
        break;
    case ALIGN_RIGHT:
        sx -= rect.width;
        break;
    }
    switch (valign)
    {
    case ALIGN_CENTER:
        sy -= rect.height / 2;
        break;
    case ALIGN_DOWN:
        sy -= rect.height;
        break;
    }
    cairo_move_to(cr, sx, sy);
    pango_cairo_show_layout(cr, layout);
    cairo_restore(cr);

    g_object_unref(layout);

    return 0;
}

/* _oOo_ */

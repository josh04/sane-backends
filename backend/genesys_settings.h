/* sane - Scanner Access Now Easy.

   Copyright (C) 2019 Povilas Kanapickas <povilas@radix.lt>

   This file is part of the SANE package.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

   As a special exception, the authors of SANE give permission for
   additional uses of the libraries contained in this release of SANE.

   The exception is that, if you link a SANE library with other files
   to produce an executable, this does not by itself cause the
   resulting executable to be covered by the GNU General Public
   License.  Your use of that executable is in no way restricted on
   account of linking the SANE library code into it.

   This exception does not, however, invalidate any other reasons why
   the executable file might be covered by the GNU General Public
   License.

   If you submit changes to SANE to the maintainers to be included in
   a subsequent release, you agree by submitting the changes that
   those changes may be distributed with this exception intact.

   If you write modifications of your own for SANE, it is your choice
   whether to permit this exception to apply to your modifications.
   If you do not wish that, delete this exception notice.
*/

#ifndef BACKEND_GENESYS_SETTINGS_H
#define BACKEND_GENESYS_SETTINGS_H

#include "genesys_enums.h"
#include "genesys_serialize.h"

struct Genesys_Settings
{
    ScanMethod scan_method = ScanMethod::FLATBED;
    ScanColorMode scan_mode = ScanColorMode::LINEART;

    // horizontal dpi
    int xres = 0;
    // vertical dpi
    int yres = 0;

    //x start on scan table in mm
    double tl_x = 0;
    // y start on scan table in mm
    double tl_y = 0;

    // number of lines at scan resolution
    unsigned int lines = 0;
    // number of pixels expected from the scanner
    unsigned int pixels = 0;
    // number of pixels expected by the frontend
    unsigned requested_pixels = 0;

    // bit depth of the scan
    unsigned int depth = 0;

    ColorFilter color_filter = ColorFilter::NONE;

    // true if scan is true gray, false if monochrome scan
    int true_gray = 0;

    // lineart threshold
    int threshold = 0;

    // lineart threshold curve for dynamic rasterization
    int threshold_curve = 0;

    // Disable interpolation for xres<yres
    int disable_interpolation = 0;

    // true is lineart is generated from gray data by the dynamic rasterization algoright
    int dynamic_lineart = 0;

    // value for contrast enhancement in the [-100..100] range
    int contrast = 0;

    // value for brightness enhancement in the [-100..100] range
    int brightness = 0;

    // cache entries expiration time
    int expiration_time = 0;

    unsigned get_channels() const
    {
        if (scan_mode == ScanColorMode::COLOR_SINGLE_PASS)
            return 3;
        return 1;
    }

    unsigned get_depth() const
    {
        if (scan_mode == ScanColorMode::LINEART)
            return 1;
        return depth;
    }

};

struct SetupParams {

    static constexpr unsigned NOT_SET = std::numeric_limits<unsigned>::max();

    // resolution in x direction
    unsigned xres = NOT_SET;
    // resolution in y direction
    unsigned yres = NOT_SET;
    // start pixel in X direction, from dummy_pixel + 1
    float startx = -1;
    // start pixel in Y direction, counted according to base_ydpi
    float starty = -1;
    // the number of pixels in X direction. Note that each logical pixel may correspond to more
    // than one CCD pixel, see CKSEL and GenesysSensor::ccd_pixels_per_system_pixel()
    unsigned pixels = NOT_SET;

    // the number of pixels in the X direction as requested by the frontend. This will be different
    // from `pixels` if the X resolution requested by the frontend is different than the actual
    // resolution. This is only needed to compute dev->total_bytes_to_read. If 0, then the value
    // is the same as pixels.
    // TODO: move the computation of total_bytes_to_read to a higher layer.
    unsigned requested_pixels = 0;

    // the number of pixels in Y direction
    unsigned lines = NOT_SET;
    // the depth of the scan in bits. Allowed are 1, 8, 16
    unsigned depth = NOT_SET;
    // the number of channels
    unsigned channels = NOT_SET;

    ScanMethod scan_method = static_cast<ScanMethod>(NOT_SET);

    ScanColorMode scan_mode = static_cast<ScanColorMode>(NOT_SET);

    ColorFilter color_filter = static_cast<ColorFilter>(NOT_SET);

    unsigned flags = NOT_SET;

    unsigned get_requested_pixels() const
    {
        if (requested_pixels != 0) {
            return requested_pixels;
        }
        return pixels;
    }

    void assert_valid() const
    {
        if (xres == NOT_SET || yres == NOT_SET || startx < 0 || starty < 0 ||
            pixels == NOT_SET || lines == NOT_SET ||depth == NOT_SET || channels == NOT_SET ||
            scan_method == static_cast<ScanMethod>(NOT_SET) ||
            scan_mode == static_cast<ScanColorMode>(NOT_SET) ||
            color_filter == static_cast<ColorFilter>(NOT_SET) ||
            flags == NOT_SET)
        {
            throw std::runtime_error("SetupParams are not valid");
        }
    }

    bool operator==(const SetupParams& other) const
    {
        return xres == other.xres &&
            yres == other.yres &&
            startx == other.startx &&
            starty == other.starty &&
            pixels == other.pixels &&
            requested_pixels == other.requested_pixels &&
            lines == other.lines &&
            depth == other.depth &&
            channels == other.channels &&
            scan_method == other.scan_method &&
            scan_mode == other.scan_mode &&
            color_filter == other.color_filter &&
            flags == other.flags;
    }
};

template<class Stream>
void serialize(Stream& str, SetupParams& x)
{
    serialize(str, x.xres);
    serialize(str, x.yres);
    serialize(str, x.startx);
    serialize(str, x.starty);
    serialize(str, x.pixels);
    serialize(str, x.requested_pixels);
    serialize(str, x.lines);
    serialize(str, x.depth);
    serialize(str, x.channels);
    serialize(str, x.scan_method);
    serialize(str, x.scan_mode);
    serialize(str, x.color_filter);
    serialize(str, x.flags);
}

struct ScanSession {
    SetupParams params;

    // whether the session setup has been computed via gl*_compute_session()
    // gl843-only
    bool computed = false;

    // specifies the reduction (if any) of hardware dpi on the Genesys chip side.
    // gl843-only
    unsigned hwdpi_divisor = 1;

    // specifies the reduction (if any) of CCD effective dpi which is performed by latching the
    // data coming from CCD in such a way that 1/2 or 3/4 of pixel data is ignored.
    unsigned ccd_size_divisor = 1;

    // the optical resolution of the scanner.
    unsigned optical_resolution = 0;

    // the number of pixels at the optical resolution.
    unsigned optical_pixels = 0;

    // the resolution of the output data.
    // gl843-only
    unsigned output_resolution = 0;

    // the number of pixels in output data
    unsigned output_pixels = 0;

    // the number of bytes in the output of a channel of a single line
    unsigned output_line_channel_bytes;

    // the number of bytes in the output of a single line
    unsigned output_line_bytes = 0;

    // the number of lines in the output of the scanner. This must be larger than the user
    // requested number due to line staggering and color channel shifting.
    unsigned output_line_count = 0;

    // the number of staggered lines (i.e. lines that overlap during scanning due to line being
    // thinner than the CCD element)
    unsigned num_staggered_lines = 0;

    // the number of lines that color channels shift due to different physical positions of
    // different color channels
    unsigned max_color_shift_lines = 0;

    // the physical pixel positions that are sent to the registers
    // gl843-only
    unsigned pixel_startx = 0;
    unsigned pixel_endx = 0;

    // the sizes of the corresponding buffers
    size_t buffer_size_read = 0;
    size_t buffer_size_lines = 0;
    size_t buffer_size_shrink = 0;
    size_t buffer_size_out = 0;

    // whether to enable ledadd functionality
    bool enable_ledadd = false;

    void assert_computed() const
    {
        if (!computed) {
            throw std::runtime_error("ScanSession is not computed");
        }
    }
};

struct Genesys_Current_Setup
{
    // pixel count expected from scanner
    int pixels = 0;
    // line count expected from scanner
    int lines = 0;

    // used exposure time
    int exposure_time = 0;
    // used xres
    float xres = 0;
    // half ccd mode
    unsigned ccd_size_divisor = 1;
    SANE_Int stagger = 0;
    //  max shift of any ccd component, including staggered pixels
    SANE_Int max_shift = 0;

    bool operator==(const Genesys_Current_Setup& other) const
    {
        return pixels == other.pixels &&
            lines == other.lines &&
            exposure_time == other.exposure_time &&
            xres == other.xres &&
            ccd_size_divisor == other.ccd_size_divisor &&
            stagger == other.stagger &&
            max_shift == other.max_shift;
    }
};

template<class Stream>
void serialize(Stream& str, Genesys_Current_Setup& x)
{
    serialize(str, x.pixels);
    serialize(str, x.lines);
    serialize(str, x.exposure_time);
    serialize(str, x.xres);
    serialize(str, x.ccd_size_divisor);
    serialize(str, x.stagger);
    serialize(str, x.max_shift);
}

#endif // BACKEND_GENESYS_SETTINGS_H
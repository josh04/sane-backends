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

#include "enums.h"
#include "serialize.h"

namespace genesys {

struct Genesys_Settings
{
    ScanMethod scan_method = ScanMethod::FLATBED;
    ScanColorMode scan_mode = ScanColorMode::LINEART;

    // horizontal dpi
    unsigned xres = 0;
    // vertical dpi
    unsigned yres = 0;

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
};

std::ostream& operator<<(std::ostream& out, const Genesys_Settings& settings);


struct SetupParams {

    static constexpr unsigned NOT_SET = std::numeric_limits<unsigned>::max();

    // resolution in x direction
    unsigned xres = NOT_SET;
    // resolution in y direction
    unsigned yres = NOT_SET;
    // start pixel in X direction, from dummy_pixel + 1
    unsigned startx = NOT_SET;
    // start pixel in Y direction, counted according to base_ydpi
    unsigned starty = NOT_SET;
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

    ScanFlag flags;

    unsigned get_requested_pixels() const
    {
        if (requested_pixels != 0) {
            return requested_pixels;
        }
        return pixels;
    }

    void assert_valid() const
    {
        if (xres == NOT_SET || yres == NOT_SET || startx == NOT_SET || starty == NOT_SET ||
            pixels == NOT_SET || lines == NOT_SET ||depth == NOT_SET || channels == NOT_SET ||
            scan_method == static_cast<ScanMethod>(NOT_SET) ||
            scan_mode == static_cast<ScanColorMode>(NOT_SET) ||
            color_filter == static_cast<ColorFilter>(NOT_SET))
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

std::ostream& operator<<(std::ostream& out, const SetupParams& params);

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

    // whether the session setup has been computed via compute_session()
    bool computed = false;

    // specifies the reduction (if any) of hardware dpi on the Genesys chip side.
    // except gl646
    unsigned hwdpi_divisor = 1;

    // specifies the reduction (if any) of CCD effective dpi which is performed by latching the
    // data coming from CCD in such a way that 1/2 or 3/4 of pixel data is ignored.
    unsigned ccd_size_divisor = 1;

    // the optical resolution of the scanner.
    unsigned optical_resolution = 0;

    // the number of pixels at the optical resolution, not including segmentation overhead.
    unsigned optical_pixels = 0;

    // the number of pixels at the optical resolution, including segmentation overhead.
    // only on gl846, g847
    unsigned optical_pixels_raw = 0;

    // the resolution of the output data.
    // gl843-only
    unsigned output_resolution = 0;

    // the number of pixels in output data (after desegmentation)
    unsigned output_pixels = 0;

    // the number of bytes in the output of a channel of a single line (after desegmentation)
    unsigned output_channel_bytes = 0;

    // the number of bytes in the output of a single line (after desegmentation)
    unsigned output_line_bytes = 0;

    // the number of bytes per line in the output data from the scanner (before desegmentation)
    // Equal to output_line_bytes if sensor does not have segments
    unsigned output_line_bytes_raw = 0;

    // the number of bytes per line as requested by the frontend
    unsigned output_line_bytes_requested = 0;

    // the number of lines in the output of the scanner. This must be larger than the user
    // requested number due to line staggering and color channel shifting.
    unsigned output_line_count = 0;

    // the total number of bytes to read from the scanner (before desegmentation)
    unsigned output_total_bytes_raw = 0;

    // the total number of bytes to read from the scanner (after desegmentation)
    unsigned output_total_bytes = 0;

    // the number of staggered lines (i.e. lines that overlap during scanning due to line being
    // thinner than the CCD element)
    unsigned num_staggered_lines = 0;

    // the number of lines that color channels shift due to different physical positions of
    // different color channels.
    unsigned max_color_shift_lines = 0;

    // actual line shift of the red color
    unsigned color_shift_lines_r = 0;
    // actual line shift of the green color
    unsigned color_shift_lines_g = 0;
    // actual line shift of the blue color
    unsigned color_shift_lines_b = 0;

    // the number of scanner segments used in the current scan
    unsigned segment_count = 1;

    // the physical pixel positions that are sent to the registers
    unsigned pixel_startx = 0;
    unsigned pixel_endx = 0;

    // certain scanners require the logical pixel count to be multiplied on certain resolutions
    unsigned pixel_count_multiplier = 1;

    // Distance in pixels between consecutive pixels, e.g. between odd and even pixels. Note that
    // the number of segments can be large.
    // only on gl124, gl846, gl847
    unsigned conseq_pixel_dist = 0;

    // The number of "even" pixels to scan. This corresponds to the number of pixels that will be
    // scanned from a single segment
    // only on gl124, gl846, gl847
    unsigned output_segment_pixel_group_count = 0;

    // The number of bytes to skip at start of line during desegmentation.
    // Currently it's always zero.
    unsigned output_segment_start_offset = 0;

    // the sizes of the corresponding buffers
    size_t buffer_size_read = 0;
    size_t buffer_size_lines = 0;
    size_t buffer_size_shrink = 0;
    size_t buffer_size_out = 0;

    // whether to enable ledadd functionality
    bool enable_ledadd = false;

    // what pipeline modifications are needed
    bool pipeline_needs_reorder = false;
    bool pipeline_needs_ccd = false;
    bool pipeline_needs_shrink = false;

    void assert_computed() const
    {
        if (!computed) {
            throw std::runtime_error("ScanSession is not computed");
        }
    }
};

std::ostream& operator<<(std::ostream& out, const ScanSession& session);

std::ostream& operator<<(std::ostream& out, const SANE_Parameters& params);

} // namespace genesys

#endif // BACKEND_GENESYS_SETTINGS_H
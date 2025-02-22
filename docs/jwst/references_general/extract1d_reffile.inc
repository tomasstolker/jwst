.. _extract1d_reffile:

EXTRACT1D Reference File
------------------------
The EXTRACT1D reference file contains information needed to guide
the 1D extraction process. It is a text file, with the information
in JSON format for Non-IFU data and in ASDF format for IFU data. 

.. include:: ../references_general/extract1d_selection.inc

.. include:: ../includes/standard_keywords.inc

Type Specific Keywords for EXTRACT1D
++++++++++++++++++++++++++++++++++++
In addition to the standard reference file keywords listed above,
the following keywords are *required* in EXTRACT1D reference files,
because they are used as CRDS selectors
(see :ref:`extract1d_selectors`):

=========  ==============================
Keyword    Data Model Name
=========  ==============================
EXP_TYPE   model.meta.exposure.type
=========  ==============================

Reference File Format for non-IFU data
++++++++++++++++++++++++++++++++++++++
EXTRACT1D reference files for non-IFU data are text files, with the information stored in
JSON format.
All the information is specified in a list with key ``apertures``.  Each
element of this list is a dictionary, one for each aperture (e.g. a slit)
that is supported by the given reference file.  The particular dictionary
used by the step is found by matching the slit name in the science data with the
value of key ``id``.  Key ``spectral_order`` is optional, but if it is
present, it must match the expected spectral order number.

The following keys are supported for non-IFU data (see below for IFU keys).
Key ``id`` is the primary criterion for selecting which element of
the ``apertures`` list to use.  The slit name (except for a full-frame
input image) is compared with the values of ``id`` in the ``apertures``
list to select the appropriate aperture.
In order to allow the possibility of multiple
spectral orders for the same slit name, there may be more than one element
of ``apertures`` with the same value for key ``id``.  These should then be
distinguished by using the secondary selection criterion ``spectral_order``.
In this case, the various spectral orders would likely have different
extraction locations within the image, so different elements of ``apertures``
are needed in order to specify those locations.
If key ``dispaxis`` is specified, its value will be used to set the
dispersion direction within the image.  If ``dispaxis`` is
not specified, the dispersion direction will be taken from the metadata for the exposure.
Key ``region_type`` can be omitted, but if it is specified, its value must
be "target".  The remaining keys specify the characteristics of the source
and background extraction regions.

* id: the slit name, e.g. "S200A1" (string)
* spectral_order: the spectral order number (optional); this can be either
  positive or negative, but it should not be zero (int)
* dispaxis: dispersion direction, 1 for X, 2 for Y (int)
* xstart: first pixel in the horizontal direction, X (float) (0-indexed)
* xstop: last pixel in the horizontal direction, X (float) (0-indexed)
* ystart: first pixel in the vertical direction, Y (float) (0-indexed)
* ystop: last pixel in the vertical direction, Y (float) (0-indexed)
* src_coeff: polynomial coefficients for source extraction region
  limits (list of lists of float); takes precedence over start/stop values.
* bkg_coeff: polynomial coefficients for background extraction region
  limits (list of lists of float)
* independent_var: "wavelength" or "pixel" (string)
* smoothing_length: width of boxcar for smoothing background regions along
  the dispersion direction (odd int)
* bkg_fit: the type of background fit or computation (string)
* bkg_order: order of polynomial fit to background data in the cross-dispersion direction (int)
* extract_width: number of pixels in cross-dispersion direction (int)
* use_source_posn: allow the extraction limits to be adjusted based on source RA/Dec (bool)

.. note::

   All parameter values that represent pixel indexes, such as ``xstart``,
   ``xstop``, and the ``src_coeff`` and ``bkg_coeff`` coefficients, are always
   in the frame of the image being operated on, which could be a small cutout
   from a larger original image. They are also ZERO-indexed and the limits are
   inclusive (e.g. 11-15 includes 5 pixels).

See :ref:`extract-1d-for-slits` for more details on how these parameters
are used in the extraction process.

Editing JSON Reference File Format for non-IFU data
+++++++++++++++++++++++++++++++++++++++++++++++++++
The default EXTRACT1D reference file is found in CRDS. The user can download this file, modify the contents,
and use this modified file in ``extract_1d`` by specifying this modified reference file with the ``override`` option
(:ref:`override in python <override_ref_python>` or :ref:`override in strun <override_ref_strun>`). The format for
JSON files has to be exact, for example, the format of a floating-point value with a fractional portion must include
at least one decimal digit, so "1." is invalid, while "1.0" is valid. The best practice after editing a JSON reference
file is to run a JSON validator off-line, such as `https://jsonlint.com/`, and correct any format errors before using
the JSON reference file in the pipeline.


Reference File Format IFU data
++++++++++++++++++++++++++++++
For IFU data the reference files are stored as ASDF files. The extraction size
varies with wavelength. The reference file contains a set of vectors defining
the extraction size based on wavelength. These vectors are all the same size and
are defined as follows: 

* wavelength: wavelength in microns. 
* radius: the radius of the circular extraction aperture (arcseconds, float)
* inner_bkg: of the inner edge of the background annulus (arcseconds, float)
* outer_bkg: of the outer edge of the background annulus (arcseconds, float)

  
In addition following general keys are also in the ASDF reference file:

* subtract_background: if true, subtract a background determined from an
  annulus with inner and outer radii given by ``inner_bkg`` and
  ``outer_bkg`` (boolean)
* method: one of "exact", "subpixel", or "center", the method
  used by photutils for computing the overlap between apertures and pixels
  (string, default is "exact")
* subpixels: if ``method`` is "subpixel", pixels will be resampled by this
  factor in each dimension (int, the default is 10)

See :ref:`extract-1d-for-ifu` for more details on how these parameters
are used in the extraction process.

Example EXTRACT1D Reference File
--------------------------------
The following JSON was taken as an example from reference file
jwst_miri_extract1d_0006.json::

    {
        "reftype": "EXTRACT1D",
        "instrument": "MIRI",
        "telescope": "JWST",
        "exp_type": "MIR_LRS-FIXEDSLIT|MIR_LRS-SLITLESS",
        "pedigree": "INFLIGHT 2022-06-01 2022-12-01",
        "description": "MIRI LRS extraction params for in-flight data",
        "author": "K.Murray",
        "history": "2024-Jun-25",
        "useafter": "2022-01-01T00:00:00",
        "apertures": [
            {
                "id": "MIR_LRS-FIXEDSLIT",
                "region_type": "target",
                "bkg_order": 0,
                "dispaxis": 2,
                "xstart": 27,
                "xstop": 34
            },
            {
                "id": "MIR_LRS-SLITLESS",
                "region_type": "target",
                "bkg_order": 0,
                "dispaxis": 2,
                "xstart": 30,
                "xstop": 41,
                "use_source_posn": false
            }
        ]
    }

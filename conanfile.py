from conan import ConanFile
from conan.tools.cmake import cmake_layout


class MachinesConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    default_options = {
        # No game asset uses AVIF; the codec pulls in the heavyweight libaom.
        "sdl_image/*:with_avif": False,
        # Only swscale (and its avutil dependency) is used, for video scaling.
        # Disable the other components and everything they would pull in
        # (codecs, network, hardware acceleration); options that do not exist
        # on the current platform are ignored.
        "ffmpeg/*:avcodec": False,
        "ffmpeg/*:avdevice": False,
        "ffmpeg/*:avfilter": False,
        "ffmpeg/*:avformat": False,
        "ffmpeg/*:swresample": False,
        "ffmpeg/*:postproc": False,
        "ffmpeg/*:with_programs": False,
        "ffmpeg/*:with_ssl": False,
        "ffmpeg/*:with_zlib": False,
        "ffmpeg/*:with_bzip2": False,
        "ffmpeg/*:with_lzma": False,
        "ffmpeg/*:with_libiconv": False,
        "ffmpeg/*:with_freetype": False,
        "ffmpeg/*:with_openjpeg": False,
        "ffmpeg/*:with_openh264": False,
        "ffmpeg/*:with_opus": False,
        "ffmpeg/*:with_vorbis": False,
        "ffmpeg/*:with_libx264": False,
        "ffmpeg/*:with_libx265": False,
        "ffmpeg/*:with_libvpx": False,
        "ffmpeg/*:with_libmp3lame": False,
        "ffmpeg/*:with_libfdk_aac": False,
        "ffmpeg/*:with_libwebp": False,
        "ffmpeg/*:with_libsvtav1": False,
        "ffmpeg/*:with_libaom": False,
        "ffmpeg/*:with_libdav1d": False,
        "ffmpeg/*:with_libalsa": False,
        "ffmpeg/*:with_pulse": False,
        "ffmpeg/*:with_vaapi": False,
        "ffmpeg/*:with_vdpau": False,
        "ffmpeg/*:with_vulkan": False,
        "ffmpeg/*:with_xcb": False,
        "ffmpeg/*:with_xlib": False,
        "ffmpeg/*:with_appkit": False,
        "ffmpeg/*:with_avfoundation": False,
        "ffmpeg/*:with_coreimage": False,
        "ffmpeg/*:with_audiotoolbox": False,
        "ffmpeg/*:with_videotoolbox": False,
    }

    def requirements(self):
        self.requires("sdl/3.4.8")
        self.requires("sdl_image/3.4.4")
        self.requires("glew/2.2.0")
        self.requires("glm/1.0.3")
        self.requires("openal-soft/1.24.3")
        self.requires("enet/1.3.18")
        self.requires("freetype/2.14.3")
        # swscale: optional video scaling support
        ffmpeg_options = {}
        if self.settings.os == "Windows" and str(self.settings.compiler) == "msvc":
            # The conancenter recipe does not archive the nasm-built objects
            # into the MSVC static libraries, leaving ff_hscale*_sse2 & co.
            # unresolved at link time. Fall back to the C implementations.
            ffmpeg_options = {"with_asm": False}
        self.requires("ffmpeg/8.1.2", options=ffmpeg_options)
        # openal-soft pins libalsa/1.2.10 exactly, while pulseaudio and sdl pull
        # the newer libalsa via a version range (resolving to 1.2.13). Align the
        # whole graph on one version to resolve the Conan conflict on Linux.
        if self.settings.os == "Linux":
            self.requires("libalsa/1.2.13", override=True)

    def layout(self):
        cmake_layout(self)

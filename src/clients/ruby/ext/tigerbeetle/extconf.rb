require "mkmf"

module TigerBeetle
  module ExtConf
    class << self
      def configure
        configure_cross_compilers
        configure_packaged_libraries
        create_makefile("tigerbeetle/tigerbeetle")
      end

      def configure_cross_compilers
        RbConfig::CONFIG["CC"] = RbConfig::MAKEFILE_CONFIG["CC"] = ENV["CC"] if ENV["CC"]
        ENV["CC"] = RbConfig::CONFIG["CC"]
      end

      def configure_packaged_libraries
        platform = detect_platform
        inc_dir = File.dirname(__FILE__) # ext/tigerbeetle/tb_client.h
        puts "inc_dir: #{inc_dir}"
        lib_dir = File.join(inc_dir, platform)
        puts "lib_dir: #{lib_dir}"
        lib_path = File.join(lib_dir, lib_file(platform))
        puts "lib_path: #{lib_path}"

        binding.irb
        abort("\nERROR: *** lib path missing ***\n\n") unless File.exist?(lib_path)


        $CPPFLAGS << " -I#{inc_dir}"
        $LDFLAGS << " -L#{lib_dir}"

        dir_config("tb_client", inc_dir, lib_dir)

        unless have_library("tb_client")
          abort("\nERROR: *** could not find tb_client development environment ***\n\n")
        end
      end

      def cross_build?
        enable_config("cross-build")
      end

      def lib_file(platform)
        if platform.include?('macos')
          'libtb_client.dylib'
        elsif platform.include?('windows')
          'tb_client.dll'
        else
          'libtb_client.so'
        end
      end

      def detect_platform
        host_os = RbConfig::CONFIG['host_os']
        cpu = RbConfig::CONFIG['host_cpu']

        # Normalize CPU architecture names
        cpu = case cpu
        when /x86_64|x64|amd64/i
          'x86_64'
        when /aarch64|arm64/i
          'aarch64'
        else
          cpu
        end

        # Detect OS and specific variant
        if host_os =~ /darwin|mac os/i
          "#{cpu}-macos"
        elsif host_os =~ /mingw|mswin|windows/i
          "#{cpu}-windows"
        elsif host_os =~ /linux/i
          # Check for musl vs glibc
          if system('ldd --version 2>&1 | grep -q musl')
            "#{cpu}-linux-musl"
          else
            # For glibc systems, try to get the version
            glibc_version = `ldd --version 2>&1 | head -n 1`
            if glibc_version =~ /2\.27/
              "#{cpu}-linux-gnu.2.27"
            else
              "#{cpu}-linux-gnu.2.27" # Default to 2.27 if can't determine version
            end
          end
        else
          abort("\nERROR: *** unsupported OS #{host_os} ***\n\n")
        end
      end
    end
  end
end

TigerBeetle::ExtConf.configure
=begin
def detect_platform
end

platform = detect_platform
if platform.nil?
abort "Unsupported platform: #{RbConfig::CONFIG['host_os']} / #{RbConfig::CONFIG['host_cpu']}"
end

puts "Detected platform: #{platform}"

lib_dir = File.join(File.dirname(__FILE__), platform)
abort "Lib dir missing" unless Dir.exist?(lib_dir)

$LDFLAGS << " -Wl,-rpath,#{lib_dir}"
$LDFLAGS << " -L#{lib_dir}"

lib_file = if platform.include?('macos')
File.join(lib_dir, 'libtb_client.dylib')
elsif platform.include?('windows')
File.join(lib_dir, 'tb_client.dll')
else
File.join(lib_dir, 'libtb_client.so')
end

abort "#{lib_file} not found" unless File.exist?(lib_file)

dir_config("tb_client", nil, lib_dir)

have_library("tb_client") or abort "tb_client library not found"

create_makefile("tigerbeetle/tigerbeetle")
=end

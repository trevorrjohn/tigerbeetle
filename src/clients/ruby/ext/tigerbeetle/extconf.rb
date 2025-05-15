require "mkmf"
require "rbconfig"

dir_config("tigerbeetle")

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
    nil # Unknown platform
  end
end

platform_dir = detect_platform
if platform_dir.nil?
  abort "Unsupported platform: #{RbConfig::CONFIG['host_os']} / #{RbConfig::CONFIG['host_cpu']}"
end

puts "Detected platform: #{platform_dir}"

lib_dir = File.join(File.dirname(__FILE__), platform_dir)
abort "Lib dir missing" unless Dir.exist?(lib_dir)

lib_file = if platform_dir.include?('macos')
  File.join(lib_dir, 'libtb_client.dylib')
elsif platform_dir.include?('windows')
  File.join(lib_dir, 'tb_client.dll')
else
  File.join(lib_dir, 'libtb_client.so')
end

abort "#{lib_file} not found" unless File.exist?(lib_file)

dir_config("tb_client", nil, lib_dir)

have_library("tb_client") or abort "tb_client library not found"

create_makefile("tigerbeetle/tigerbeetle")


# homebrew formula for Mac OS install -- see http://mxcl.github.com/homebrew/

require 'formula'

class Ostn02c < Formula
  homepage 'https://github.com/jawj/OSTN02C'
  url 'https://github.com/jawj/OSTN02C/tarball/v0.1.2'
  sha256 'b02994febc9174a1274d53718f13a9874b8eaaddf925a0707b30ad8c38d7053e'
  
  def install
    system "#{ENV.cc} OSTN02/*.c -std=gnu99 -D_GNU_SOURCE -lm -Wall -O2 -o ostn02c"
    bin.install "ostn02c"
  end
  
  def test
    system "ostn02c test"
  end
end


# homebrew formula for Mac OS install -- see http://mxcl.github.com/homebrew/

require 'formula'

class Ostn02c < Formula
  homepage 'https://github.com/jawj/OSTN02C'
  url 'https://github.com/jawj/OSTN02C/tarball/v0.1.2'
  md5 '0f25f9a41eec1b40d2201579722a9684'
  
  def install
    system "#{ENV.cc} OSTN02/*.c -std=gnu99 -D_GNU_SOURCE -lm -Wall -O2 -o ostn02c"
    bin.install "ostn02c"
  end
  
  def test
    system "ostn02c test"
  end
end

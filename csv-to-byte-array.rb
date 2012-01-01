#!/usr/bin/env ruby
%w{CSV}.each { |lib| require lib }
File.open("OSTN02.data", "wb") do |data_file|
  File.open("OSTN02.byte-array", "w") do |array_file|
    array_file.puts("const unsigned char OSTN02Data[] = {")
    CSV.foreach("OSTN02_OSGM02_GB.txt") do |csv_row|
      dflag = csv_row[6].to_i
      if dflag == 0
        eshift = nshift = gshift = 0
        else
        eshift = csv_row[3].sub('.', '').to_i - 86000
        nshift = csv_row[4].sub('.', '').to_i + 82000
        gshift = csv_row[5].sub('.', '').to_i - 43000
      end
      arr = [eshift, nshift, gshift, dflag]
      binstr = arr.pack('SSSC')
      data_file.write(binstr)
      array_file.puts(binstr.each_char.map { |c| "#{c.ord}," }.join)
    end
    array_file.puts("};")
  end
end
File.open('OSTN02.data.md5', 'w')  { |f| f.write(`md5 -q OSTN02.data`) }

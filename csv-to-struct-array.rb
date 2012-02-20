#!/usr/bin/env ruby
# encoding: utf-8

# this script processes the OSTN02/OSGM02 data in OS-supplied CSV format
# into a packed array of structs, and an index thereto

require 'CSV'
puts 'loading'
data = []
last_n = nil
CSV.foreach("OSTN02_OSGM02_GB.txt") do |csv_row|
  e     = csv_row[1].to_i / 1000
  n     = csv_row[2].to_i / 1000
  dflag = csv_row[6].to_i
  if dflag == 0
    eshift = nshift = gshift = 0
  else
    # these offsets (-86000, 82000, -43000) minimise the number of bits required to store
    eshift = csv_row[3].sub('.', '').to_i - 86000  
    nshift = csv_row[4].sub('.', '').to_i + 82000
    gshift = csv_row[5].sub('.', '').to_i - 43000
  end
  if n != last_n
    data.push([]) 
    print n.to_s + ' '
  end
  data.last.push(e: e, n: n, dflag: dflag, eshift: eshift, nshift: nshift, gshift: gshift)
  last_n = n
end

puts "\n\ntrimming"
data.each do |row|
  print row[0][:n].to_s + ' '
  first_non_zero_index = row.find_index { |cell| cell[:dflag] != 0 }
  if first_non_zero_index.nil?
    row.clear
    next
  end
  row.slice!(0...first_non_zero_index)
  last_non_zero_index = row.length - row.reverse_each.find_index { |cell| cell[:dflag] != 0 }  # actually the index of the following zero
  row.slice!(last_non_zero_index..row.length)
  print row.length.to_s + ', '
end

c_notice = %{
// OSTN02/OSGM02 data (derived from OSTN02_OSGM02_GB.txt in http://www.ordnancesurvey.co.uk/oswebsite/gps/docs/OSTN02_OSGM02files.zip)
// © Crown copyright 2002. All rights reserved

}

puts "\n\nwriting"
indices_written = data_written = 0
File.open("shifts.index.data", "w") do |index_file|
  index_file.puts(c_notice + 'static const OSTN02Index OSTN02Indices[] = {')
  File.open("shifts.data", "w") do |data_file|
    data_file.puts(c_notice + 'static const OSTN02Datum OSTN02Data[] = {')
    data.each do |row|
      print row.empty? ? '- ' : row[0][:n].to_s + ' '
      emin = row.empty? ? 0 : row.first[:e]
      index_file.write(indices_written == 0 ? ' ' : ',')
      index_file.puts("{#{emin},#{row.length},#{data_written}}")
      indices_written += 1
      row.each do |cell|
        data_file.write(data_written == 0 ? ' ' : ',')
        data_file.puts("{#{cell[:eshift]},#{cell[:nshift]},#{cell[:gshift]},#{cell[:dflag]}}")
        data_written += 1
      end
    end
    data_file.puts('};')
  end 
  index_file.puts('};')
end

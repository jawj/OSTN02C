#!/usr/bin/env ruby
%w{CSV}.each { |lib| require lib }
first = true
File.open("OSTN02.struct-array", "w") do |array_file|
  array_file.puts("const OSTN02Record OSTN02Records[] = {")
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
    array_file.write(first ? " " : ",")
    array_file.puts("{#{eshift},#{nshift},#{gshift},#{dflag}}")
    first = false
  end
  array_file.puts("};")
end

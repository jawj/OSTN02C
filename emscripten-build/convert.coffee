
(self[id + 'Field'] = get {id}) for id in ['e', 'n', 'mslAlt', 'datum', 'lat', 'lon', 'gpsAlt', 'dms', 'dec']
osgbFields = [eField,   nField,   mslAltField, datumField]
gpsFields  = [lonField, latField, gpsAltField]
degFmt = get(id: 'inputs').elements.degFmt
mapCanvas = get(id: 'mapCanvas')
mapCtx = mapCanvas.getContext '2d'
arrow = get id: 'arrow'

parseLatOrLon = (s, isLat) -> 
  dec = s.match /^\s*-?([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)\s*$/
  if dec then value = parseFloat s
  else
    dms = s.match ///
                  ^\s*
                  ([NESW])?  # compass direction
                  \s*
                  ([0-9]+)  # degrees
                  (
                    (
                      [^0-9]+   # separator
                      ([0-9]+)  # minutes
                    )
                    (
                      [^0-9]+   # separator
                      ([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)  # seconds, with optional decimal
                    )?  # seconds optional if minutes given
                  )?  # minutes and seconds optional
                  [^NESW0-9]*
                  ([NESW])?  # compass direction
                  \s*$
                  ///i

    if not dms then return null

    deg = parseInt dms[2]
    min = parseInt (dms[5] ? '0')
    sec = parseFloat (dms[7] ? '0')

    if min >= 60 or sec >= 60 then return null
    if (not dms[1]?) and (not dms[8]?) then return null
    if (dms[1]? and dms[8]?) then return null

    dir = (dms[1] ? dms[8]).toUpperCase()
    if (isLat and dir not in ['N', 'S']) or ((not isLat) and dir not in ['E', 'W']) then return null
    
    value = OSTN02C.decimalFromDegMinSec {deg, min, sec, westOrSouth: dir in ['W', 'S']}

  maxVal = if isLat then 90 else 180
  if value < -maxVal or value > maxVal then return null
  return value

parseEorN = (s) ->
  validFormat = s.match /^\s*-?([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)\s*$/
  return if validFormat then parseFloat s else null

parseAlt = (s) ->
  validFormat = s.match /^\s*-?([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)\s*m?\s*$/
  return if validFormat then parseFloat s else null

formatLatOrLon = (v, isLat) ->  # TODO: leading zeroes on mins and secs
  if degFmt.value is 'dms'
    dms = OSTN02C.degMinSecFromDecimal v
    dir = if isLat then (if dms.westOrSouth then 'S' else 'N') else (if dms.westOrSouth then 'W' else 'E')
    "#{dms.deg}\u00b0\u2008#{dms.min}\u2032\u2008#{dms.sec.toFixed 2}\u2033 #{dir}"
  else
    v.toFixed 6

formatEOrN = (v) -> v.toFixed 0
formatAlt = (alt) -> (Math.round alt) + 'm'
showMissing = (inputs) -> (input.value = "—") for input in inputs

displayGps = (latLon) ->
  if latLon?
    latField.value    = formatLatOrLon latLon.lat, yes
    lonField.value    = formatLatOrLon latLon.lon, no
    gpsAltField.value = formatAlt latLon.elevation
    drawMap yes
  else
    showMissing gpsFields
    drawMap no

displayOsgb = (en) ->
  displayGeoid en
  if en?
    eField.value      = formatEOrN en.e
    nField.value      = formatEOrN en.n
    mslAltField.value = formatAlt en.elevation 
    drawMap yes
  else
    showMissing osgbFields
    drawMap no

displayGeoid = (en) ->
  if en?
    geoidName = OSTN02C.geoidNameForIndex en.geoid
    geoidRegion = OSTN02C.geoidRegionForIndex en.geoid
    geoidCaption = geoidRegion + if geoidName is geoidRegion then '' else " (#{geoidName})"
    datumField.innerHTML = geoidCaption
  else 
    datumField.innerHTML = "—"

osgbChanged = ->
  strs = (input.value for input in osgbFields[0..2])
  values = ((if i is 2 then parseAlt s else parseEorN s) for s, i in strs)
  (cls input, if values[i]? then remove: 'invalid' else add: 'invalid') for input, i in osgbFields[0..2]
  if null in values
    displayGps null
    displayGeoid null
    return

  [e, n, elevation] = values
  enETRS89 = OSTN02C.ETRS89EastingNorthingFromOSGB36EastingNorthing {e, n, elevation, geoid: 0}
  if enETRS89.geoid is 0
    displayGps null
    displayGeoid enETRS89
    return

  displayGeoid enETRS89
  displayGps OSTN02C.ETRS89LatLonFromETRS89EastingNorthing enETRS89
  cls arrow, remove: 'reverse'
  saveData 'coords', {changed: 'osgb', values: strs}

gpsChanged = ->
  strs = (input.value for input in gpsFields)
  values = ((if i is 2 then parseAlt s else parseLatOrLon s, (i is 1)) for s, i in strs)
  (cls input, if values[i]? then remove: 'invalid' else add: 'invalid') for input, i in gpsFields
  if null in values
    displayOsgb null
    return

  [lon, lat, elevation] = values
  en = OSTN02C.OSGB36EastingNorthingFromETRS89EastingNorthing OSTN02C.ETRS89EastingNorthingFromETRS89LatLon {lat, lon, elevation}
  if en.geoid is 0
    displayOsgb null
    displayGeoid en
    return

  displayOsgb en
  cls arrow, add: 'reverse'
  saveData 'coords', {changed: 'gps', values: strs}

dmsChanged = ->
  for field, i in [lonField, latField]
    isLat = i is 1
    value = parseLatOrLon field.value, isLat
    if value then field.value = formatLatOrLon value, isLat

  saveData 'degFmt', degFmt.value

drawMap = (xMarksTheSpot) ->
  mapCtx.clearRect 0, 0, mapCanvas.width, mapCanvas.height
  mapCtx.globalAlpha = 0.5
  mapCtx.drawImage gbMap, 10, 10

  if xMarksTheSpot
    e = parseFloat eField.value
    n = parseFloat nField.value
    x = 10 + e / 1500
    y = -10 + mapCanvas.height - n / 1500

    mapCtx.globalAlpha = 1.0
    mapCtx.beginPath()
    mapCtx.moveTo x - 10, y
    mapCtx.lineTo x + 10, y
    mapCtx.moveTo x, y - 10
    mapCtx.lineTo x, y + 10
    mapCtx.lineWidth = 4
    mapCtx.strokeStyle = '#fff'
    mapCtx.stroke()

saveData = (k, v) ->
  self.localStorage?.setItem k, (JSON.stringify v)

loadData = (k) ->
  JSON.parse (self.localStorage?.getItem k)

(input.oninput = osgbChanged) for input in osgbFields
(input.oninput = gpsChanged)  for input in gpsFields
decField.onclick = dmsField.onclick = dmsChanged

OSTN02C = OSTN02CFactory null, null, null, ->
  prevDegFmt = (loadData 'degFmt') ? 'dec'
  degFmt.value = prevDegFmt

  prevData = (loadData 'coords') ? {changed: 'osgb', values: ['530450', '105593', '67m']}
  fields = if prevData.changed is 'osgb' then osgbFields else gpsFields

  (fields[i].value = value) for value, i in prevData.values
  if prevData.changed is 'osgb' then osgbChanged() else gpsChanged()
  dmsChanged()

gbMap = make tag: 'img', src: 'data:image/png;base64,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'



OSTN02C = OSTN02CFactory()

(self[id + 'Field'] = get {id}) for id in ['e', 'n', 'mslAlt', 'datum', 'lat', 'lon', 'gpsAlt']
osgbFields = [eField,   nField,   mslAltField, datumField]
gpsFields  = [latField, lonField, gpsAltField]
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

parseAlt = (s) ->
  validFormat = s.match /^\s*-?([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)\s*m?\s*$/
  return if validFormat then parseFloat s else null

parseEorN = (s) ->
  validFormat = s.match /^\s*-?([0-9]+|[0-9]+\.[0-9]*|[0-9]*\.[0-9]+)\s*$/
  return if validFormat then parseFloat s else null


formatLatOrLon = (v) -> v.toFixed 6
formatEOrN = (v) -> v.toFixed 0
formatAlt = (alt) -> (Math.round alt) + 'm'
showMissing = (inputs) -> (input.value = "—") for input in inputs

displayGps = (latLon) ->
  if latLon?
    latField.value    = formatLatOrLon latLon.lat
    lonField.value    = formatLatOrLon latLon.lon
    gpsAltField.value = formatAlt latLon.elevation
  else
    showMissing gpsFields

displayOsgb = (en) ->
  displayGeoid en
  if en?
    eField.value      = formatEOrN en.e
    nField.value      = formatEOrN en.n
    mslAltField.value = formatAlt en.elevation 
  else
    showMissing osgbFields

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
    displayGeoid null
    return

  displayGeoid enETRS89
  displayGps OSTN02C.ETRS89LatLonFromETRS89EastingNorthing enETRS89

  arrow.style.visibility = 'visible'
  arrow.style.transform = 'scaleX(1)'

gpsChanged = ->
  strs = (input.value for input in gpsFields)
  values = ((if i is 2 then parseAlt s else parseLatOrLon s) for s, i in strs)
  (cls input, if values[i]? then remove: 'invalid' else add: 'invalid') for input, i in gpsFields
  if null in values
    displayOsgb null
    return

  [lat, lon, elevation] = values
  en = OSTN02C.OSGB36EastingNorthingFromETRS89EastingNorthing OSTN02C.ETRS89EastingNorthingFromETRS89LatLon {lat, lon, elevation}
  if en.geoid is 0
    displayOsgb null
    return

  displayOsgb en

  arrow.style.visibility = 'visible'
  arrow.style.transform = 'scaleX(-1)'


(input.oninput = osgbChanged) for input in osgbFields
(input.oninput = gpsChanged)  for input in gpsFields


# concise interface to getElementById, getElementsByTagName, getElementsByClassName
# opts: either id *or* tag and/or cls, [inside], [multi]
# id is an id, tag is a tag name
# cls is a space-separated list of class names: element must satisfy all (plus tag, if present)
# if id or a unique tag (body, head, etc) specified, returns one element, else returns an array
# depends on: cls

get = (opts = {}) ->  
  inside = opts.inside ? document
  tag = opts.tag ? '*'
  if opts.id?
    return inside.getElementById opts.id
  hasCls = opts.cls?
  if hasCls and tag is '*' and inside.getElementsByClassName?
    return inside.getElementsByClassName opts.cls
  els = inside.getElementsByTagName tag
  if hasCls then els = (el for el in els when cls el, has: opts.cls)
  if not opts.multi? and tag.toLowerCase() in get.uniqueTags then els[0] ? null else els

get.uniqueTags = 'html body frameset head title base'.split(' ')


# concise creating, setting attributes and appending of elements
# opts: tag, parent, prevSib, text, cls, [attrib]
# depends on: text

make = (opts = {}) -> 
  t = document.createElement opts.tag ? 'div'
  for own k, v of opts
    switch k
      when 'tag' then continue
      when 'parent' then v.appendChild t
      when 'kids' then t.appendChild c for c in v when c?
      when 'prevSib' then v.parentNode.insertBefore t, v.nextSibling
      when 'text' then t.appendChild text v
      when 'cls' then t.className = v
      else t[k] = v
  t

text = (t) -> document.createTextNode '' + t


# easy className-wrangling: both querying and modifying
# opts: either has *or* add and/or remove and/or toggle
# all opt values are space-separated lists of class names
# depends on: none

cls = (el, opts = {}) ->
  classHash = {}  
  classes = el.className.match(cls.re)
  if classes?
    (classHash[c] = yes) for c in classes
  hasClasses = opts.has?.match(cls.re)
  if hasClasses?
    (return no unless classHash[c]) for c in hasClasses
    return yes
  addClasses = opts.add?.match(cls.re)
  if addClasses?
    (classHash[c] = yes) for c in addClasses
  removeClasses = opts.remove?.match(cls.re)
  if removeClasses?
    delete classHash[c] for c in removeClasses
  toggleClasses = opts.toggle?.match(cls.re)
  if toggleClasses?
    for c in toggleClasses
      if classHash[c] then delete classHash[c] else classHash[c] = yes
  el.className = (k for k of classHash).join ' '
  null

cls.re = /\S+/g


# simple JSON-P cross-site JS requests
# opts: url, callback, [callbackName]
# unless specifying callbackName, the url callback parameter within url should be passed as '<cb>'
# depends on: make, get

jsonp = (opts) ->
  callbackName = opts.callback ? '_JSONPCallback_' + jsonp.callbackNum++
  url = opts.url.replace '<cb>', callbackName
  window[callbackName] = opts.success ? jsonp.noop
  make tag: 'script', src: url, parent: (get tag: 'head')

jsonp.callbackNum = 0
jsonp.noop = ->


# easy XHR
# not much tested
# depends on: none

xhr = (opts = {}) ->  # for IE6, and serious cross-browser wrangling, use jQuery!
  method = opts.method ? 'GET'
  req = new XMLHttpRequest()
  req.onreadystatechange = -> 
    if req.readyState is 4 and (req.status is 200 or not location.href.match /^https?:/)
      opts.success(req)
  req.overrideMimeType 'text/plain; charset=x-user-defined' if opts.binary
  req.overrideMimeType opts.mime if opts.mime?
  req.user = opts.user if opts.user?
  req.password = opts.password if opts.password?
  req.setRequestHeader k, v for k, v of opts.headers if opts.headers?
  req.open method, opts.url
  req.send opts.data
  yes
  

# convert string to/from UTF8 (binary/byte) string
# see: http://ecmanaut.blogspot.co.uk/2006/07/encoding-decoding-utf8-in-javascript.html

UTF8 =
  enc: (s) -> unescape (encodeURIComponent s)
  dec: (s) -> decodeURIComponent (escape s)


# optimised Base64 encoder/decoder (by removing conditionals from the main loop)
# slower originals: https://github.com/rwz/base64.coffee/blob/master/base64.coffee
# in Chrome, the string concatenation version is about 33% quicker than an array + join strategy
# in Firefox and Safari, array + join strategy (with preallocated array) is 5 - 10%+ quicker

B64 =
  chars: 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='.split ''
  nonchars: new RegExp '[^A-Za-z0-9+/=]+', 'g'  # CoffeeScript struggles with a slash in a regex literal

  enc: (input, output = '') ->
    chars = B64.chars
    len = input.length
    padLen = [0, 2, 1][len % 3]
    padded = if padLen is 0 then input else input + '\x00\x00'.substring(0, padLen)
    i = 0

    while i < len
      chr1 = padded.charCodeAt(i++) & 255
      chr2 = padded.charCodeAt(i++) & 255
      chr3 = padded.charCodeAt(i++) & 255
      output += chars[chr1 >> 2]
      output += chars[((chr1 & 3) << 4) | (chr2 >> 4)]
      output += chars[((chr2 & 15) << 2) | (chr3 >> 6)]
      output += chars[chr3 & 63]
    if padLen is 0 then output 
    else output.substring(0, output.length - padLen) + '=='.substring(0, padLen)
    
  dec: (input, sanitize = yes, output = '') ->
    if sanitize then input = input.replace B64.nonchars, ''
    charmap = B64.charmap
    len = input.length
    i = 0

    while i < len
      enc1 = charmap[input.charAt(i++)]
      enc2 = charmap[input.charAt(i++)]
      enc3 = charmap[input.charAt(i++)]
      enc4 = charmap[input.charAt(i++)]

      chr1 = (enc1 << 2) | (enc2 >> 4)
      chr2 = ((enc2 & 15) << 4) | (enc3 >> 2)
      chr3 = ((enc3 & 3) << 6) | enc4

      output += String.fromCharCode chr1, chr2, chr3

    if enc4 isnt 64 then output
    else output.substring(0, output.length - if enc3 is 64 then 2 else 1)

B64.charmap = {}
(B64.charmap[char] = i) for char, i in B64.chars
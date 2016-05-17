
window.scrollTo ?= window.scroll

outNode   = document.getElementById 'output'
parseNode = document.createElement  'div'
inText    = document.getElementById 'input'
inForm    = document.getElementById 'inputForm'

printHTML = (html) ->
  parseNode.innerHTML = html
  while (child = parseNode.firstChild)
    outNode.appendChild(parseNode.removeChild child)
  setTimeout (-> window.scrollTo 0, 10e8), 0

codeMap =
  '1': 'bold'
  '7': 'inverse'
  '4': 'uline'

stdoutStr = ''

stdoutCallback = (chr) ->
  if chr isnt null then stdoutStr += String.fromCharCode chr
  if chr is 10 or chr is null
    openCodes = 0
    html = stdoutStr.replace /\x1b\[(1|22|7|27|4|24)m/g, (_, match) ->
      cls = codeMap[match]
      if cls
        openCodes++
        "<span class='#{cls}'>"
      else
        openCodes--
        "</span>"

    if openCodes is 0
      printHTML html
      stdoutStr = ''


OSTN02C = OSTN02CFactory null, stdoutCallback, null


class CommandHistory
  constructor: ->
    pastHistory = localStorage.getItem 'cmdhistory'
    @history = if pastHistory then JSON.parse(pastHistory) else [{edited: ''}]
    @index = @history.length - 1

  commandChanged: (command) ->
    @history[@index].edited = command

  commandEntered: (command) ->
    @history[@index].edited = @history[@index].entered  # return to originally entered value
    @history[@history.length - 1].entered = @history[@history.length - 1].edited = command
    if command.length > 0 and command isnt @history[@history.length - 2]?.entered  # don't save blanks or dupes
      @history.push {edited: ''}
      while @history.length > 100 then @history.shift()
    else
      @history[@history.length - 1].edited = ''
    @index = @history.length - 1
    localStorage.setItem 'cmdhistory', JSON.stringify(@history)
    @history[@index].edited

  commandNavigated: (delta) ->
    @index += delta
    if @index < 0 then @index = 0
    if @index > @history.length - 1 then @index = @history.length - 1
    @history[@index].edited

commandHistory = new CommandHistory()


inForm.addEventListener 'submit', (e) ->
  e.preventDefault()
  command = inText.value
  printHTML "> #{command}\n"
  inText.value = commandHistory.commandEntered command
  args = command.match /[\S+]+/g
  if args?
    exe = args.shift()
    if exe is 'clear' then outNode.innerHTML = ''
    else if exe is 'ostn02c' then OSTN02C.callMain args
    else printHTML "#{exe}: command not found\n\n"

completions = [
  'ostn02c help'
  'ostn02c test'
  'ostn02c list-geoids'
  'ostn02c gps-to-grid '
  'ostn02c grid-to-gps '
  'clear'
]


inText.addEventListener 'keydown', (e) ->
  kc = e.keyCode

  if kc is 9   # 9 = tab
    e.preventDefault()
    matches = []
    for completion in completions
      if completion.substring(0, inText.value.length) is inText.value then matches.push completion

    if matches.length is 1 then inText.value = matches[0]
    else if matches.length > 1
      common = ''
      firstMatch = matches[0]
      for charPos in [0 ... firstMatch.length]
        for i in [1 ... matches.length] 
          same = firstMatch.charAt(charPos) is matches[i].charAt(charPos)
          break unless same
        break unless same
        common += firstMatch.charAt(charPos)
      
      if inText.value is common
        printHTML(matches.join('     ') + '\n\n')
      else
        inText.value = common


  else if kc in [38, 40]  # 38 = up, 40 = down
    e.preventDefault()
    e.target.value = commandHistory.commandNavigated (if kc is 38 then -1 else +1)


inText.addEventListener 'input', (e) ->
  commandHistory.commandChanged e.target.value


window.addEventListener 'load', ->
  inText.focus()
  printHTML 'Type <span class="bold">ostn02c help</span> below to get started.\n'
  printHTML 'This fake shell supports [Tab] completion, [Up] + [Down] command history, and two commands (<span class="bold">ostn02c</span> and <span class="bold">clear</span>).\n\n'


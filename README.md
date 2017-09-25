junix: Unix as if JSON mattered
===============================

Unix tools are built around the idea of reading and writing lines of text.
Anxieties about the limitations of this approach, and ideas for working with
more complex structures, [go back decades](http://doc.cat-v.org/bell_labs/structural_regexps/).

What if the tools were built around JSON tokens and objects instead?

Quoting conventions for arguments
---------------------------------

Ideally commmand-line arguments would also be JSON tokens or objects, but
the tools need to interoperate with the standard shell and other utilities,
and arguments are normally typeless, interpreted as strings or options.

For explicit typing,
any argument that begins with a colon has that character stripped and
applies normal JSON parsing to the remainder of the argument. So:

 * `:true`, `:false`, `:null`
 * `:"hello world"`
 * `:12345.67`
 * `:{ "true": false }`

Any argument that does *not* begin with a colon is interpreted as a number
if it is uniformly numeric (including scientific notation), and as a string
otherwise.  Strings that begin with a colon, or strings that look like numbers,
must be quoted with `:` to avoid misinterpretation.

By this standard, command line options like `-rf` and the options-terminator `--`
are strings. Perhaps this should be revisited.

Note that the argument `"hello"` (which would be typed as `'"hello"'` or `"\"hello\""`)
is equivalent to `:"\"hello\""`, with nested quotation marks.

(A future shell should also be aware of these conventions for `*` expansion
and tab completion.)

jecho
-----

`jecho` prints its arguments. By convention and for readability, each is printed
on a separate line, although they could be separated by any whitespace in practice.

```
$ jecho "hello world" ":{ \"foo\": true }" 12345
"hello world"
{"foo":true}
12345
```

jarray
------

`jarray` wraps the JSON objects that it reads in an array.

```
$ jecho "hello world" ":{ \"foo\": true }" 12345 | jarray
["hello world",{"foo":true},12345]
```

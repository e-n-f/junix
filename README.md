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

jarg
----

`jarg` stringifies its input objects using the command line argument quoting conventions,
so that output of one command can be passed as arguments to another. The idea is that

    jecho $(jwhatever | jarg)

should be equivalent to `jwhatever` on its own.

```
$ jecho "hello world" ":{ \"foo\": true }" 12345 | jarg
:"hello\u0020world" :{"foo":true} :12345
```

Note that spaces within strings are quoted to avoid creating separate shell arguments.
The only spaces in the output separate JSON objects.

jcat
----

`jcat` concatenates JSON streams, which is basically like concatenating any other streams.

jarray
------

`jarray` wraps the JSON objects that it reads in an array.

```
$ jecho "hello world" ":{ \"foo\": true }" 12345 | jarray
["hello world",{"foo":true},12345]
```

junarray
--------

`junarray` removes an array wrapper from its input. It is an error if an
input object is not an array. If there are multiple arrays in the input,
their contents are merged together.

```
$ jecho ":[1,2,3]" ":[4,5]" | junarray
1
2
3
4
5
```

jkeys
-----

`jkeys` extracts the keys of each hash in its input into an array. It is an error
if an input object is not a hash.

```
$ jecho ":{\"foo\": true, \"bar\": false}" ":{\"yes": \"no\"}"
["foo","bar"]
["yes"]
```

jls
---

It might be nice if the output of plain `jls` were just a stream of strings, but this
interacts badly with recursive or long listings. So instead they are hash keys, and
if you just want the names, you can use `jkeys`.

```
$ jls / | jkeys
"bin"
"dev"
"etc"
"usr"
"tmp"
```

```
$ jls -l /
{
	"bin": { "user": "root", "group": "wheel", "bytes": 1326, "modified": "2016-08-01T12:34:45Z", "type": "directory" },
	"dev": { "user": "root", "group": "wheel", "bytes": 4218, "modified": "2016-08-29T11:22:33Z", "type": "directory" },
	...
}
```

```
$ jls -r /
{
	"bin": {
		"entries": {
			"bash":{}, "cat":{}, "chmod":{}, "cp":{}, "date":{}, ...
		}
	},
	"usr": {
		"entries": {
			"bin": {
				"entries": { "awk":{}, "bc":{}, "c++":{}, ... }
			},
			"include": {
				"entries": { "stdio.h":{}, "stdlib.h":{}, "unistd.h":{}, ... }
			},
			...
		}
	},
	...
}
```

TBD: How should tersely or verbosely should permissions be reported?

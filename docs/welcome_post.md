Hello everyone,

I have written a python regular expression engine called “SAR” which takes a different approach from current regexp engines by providing an event driven regexp engine — this means that for each regexp we want to match, we also append a callback which will be called upon it’s match on the text in the same order in which the matched text occurred in the text, allowing us to handle that match knowing exactly what regexp was caught and append context into that regexp match, which gives us much more flexibility in writing regexps.

When SAR was created, it was designed with emphasize on the following points:

1. It is built to handle many regexps together (can handle very large numbers, above millions) to allow very high performance while also keeping track of which regexp was caught — which differs from alternation since you cannot know which regexp was caught among multiple regexps.
2. also, it seems to perform much faster then python native regexp engine with alternation (on benchmark running on my own pc, I’ve managed to get in SAR more then 150x times better performance then in python native regexp engine (code is in the post I link below).
3. It is designed to offer a much better interactivity with the regexp engine itself, while current regexp engines barely allow us to interact with them, SAR embraces the interactions during the matching process, every time there is a match, a handler will be called which is unique for that specific regexp that was caught allowing us to decide what we want to do with that match and thus bring much more flexibility to us

With SAR, we are now able to append context to our regular expressions, which was previously very hard to achieve, and aside of the performance boost, this alone could prove itself very useful.

for more information, please read my post explaining SAR in greater detail:

https://github.com/nmnsnv/regexp_sar/blob/master/docs/introduction.md
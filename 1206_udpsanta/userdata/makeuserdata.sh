#!/bin/bash

if [ "$FLAGFILE" == "" ];
then
	echo "Specify flag file in the FLAGFILE env variable!"
	exit 1
fi

flag=$(cat $FLAGFILE)

flaguser="Jessica"
pwnssluser="Timmy"
sintuser="Steven"

keysize=2048
userlist=$(echo TheRealSanta $sintuser $pwnssluser Emily Chloe $flaguser Jack William Oliver Charlotte Lucy | tr " " "\n" | shuf)

cat > wishes.txt <<EOF
a pony
an XBox
a playstation
a bicycle
world peace
5 stock market shares of Apple
bitcoins
one million euros
a laptop
a cake
a videogame
more CTF challenges
a tropical island
a doll
a car
an airplane
a jukebox
some new shoes
a dog
a cat
a turtle
a parrot
EOF

mkdir -p msgs keys

echo "creating userlist.txt"
(
for i in $userlist;
do
	echo $i
done
) > userlist.txt

for u in $userlist;
do
	echo "Generating keys for $u..."
	tr -cd "a-zA-Z0-9" </dev/urandom | head -c 16 > keys/$u.iv
	tr -cd "a-zA-Z0-9" </dev/urandom | head -c 32 > keys/$u.key

	if [ "$u" == "$pwnssluser" ];
	then
		cp CVE-2008-0166.priv keys/$u.priv
	else
		openssl genrsa -out keys/$u.priv $keysize
	fi
	openssl rsa -in keys/$u.priv -pubout > keys/$u.pub
done

counter=0
for u in $userlist;
do
	echo "Generating messages for $u..."
	santa="Santa"
	wish=$(shuf wishes.txt | head -1)

	if [ "$u" == "$flaguser" ];
	then
		wish="the flag for the udpsanta challenge"
	fi

	if [ "$u" == "$sintuser" ];
	then
	    	santa="Sinterklaas"
	fi

	if [ "$u" != "TheRealSanta" ];
	then
		echo -en "$u" > msgs/TheRealSanta.$counter.from
		echo -en "$RANDOM" > msgs/TheRealSanta.$counter.id
		echo -en "Dear $santa,\n\nI have behaved exceptionally well this year\nand I would really like $wish for Christmas.\nWould that be possible?\n\nYour biggest fan,\n-- $u\n" > msgs/TheRealSanta.$counter.msg

		reply="I will do my best to arrange that!"
		if [ "$u" == "$flaguser" ];
		then
			reply="The flag is $flag."
		fi
		echo -en "TheRealSanta" > msgs/$u.0.from
		echo -en "$RANDOM" > msgs/$u.0.id
		echo -en "Hello there $u,\n\nI see in my book that you have not been naughty this year,\nand you say you would like $wish for Christmas?\nOf course! $reply\n\nAll my best wishes,\nHo Ho Ho,\n-- Santa Claus\n" > msgs/$u.0.msg
		if [ "$u" == "$pwnssluser" ];
		then
			echo -en "\nPS: My heart bleeds to see you use such a weak key.\nI sure hope that this server is better\nprotected than that...\n" >> msgs/$u.0.msg
		fi
		if [ "$u" == "$sintuser" ];
		then
			echo -en "\nPS: My name is Santa Claus, not Sinterklaas: that is my cousin.\nSay hi to him when you get in contact!\nIt's his birthday today...\n" >> msgs/$u.0.msg
		fi


		counter=$((counter + 1))
	fi
done

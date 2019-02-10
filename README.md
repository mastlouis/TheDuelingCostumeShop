# TheDuelingCostumeShop

Matthew's Ideas (Just to gather them into one place):

-If we measure time in seconds, and each second is meant to be indivisible, then we should have each person sleep for 1 second every time they fail to get the lock.
	-Each person should keep track of the length of time for which they've waited in a stack variable. This makes time relative, which is weird, but should ultimately be okay if things move much faster than seconds.

-We can keep a variable in the struct tracking how many pirates and ninjas are currently waiting to get into the shop. This can help the shop decide whether or not it needs switch from pirates to ninjas.

	-A pirate or ninja can tell whether or not it needs to remove itself from this count by looking at the amount of time it's waited (whether or not it's zero).

-We should have a variable in the struct to track the number of available teams of costumers.


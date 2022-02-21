def readpro(profil, ntactoski, ntac, nneu):
	"""Read SNN protocol file profil beginning from ntactoski."""
	pro = [[] for i in range(nneu)]
	with open(profil, 'r') as fil:
		for i in range(ntactoski):
			fil.readline()
		for i in range(ntac):
			a = fil.readline()
			if len(a) < nneu:
				break
			for j in range(nneu):
				if a[j] == '@':
					pro[j].append(i)
	return pro


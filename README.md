# IAJV_RAVEN

# Equipe
Desrousseaux Florian
Dupont François
Edorh François
Guison Vianney
Stievenard Thomas 

#Ajouts

- Réseau neuronal
< ... >

- Logique floue
  Weapon_RocketLauncher:
			 > 5 intervalles au lieu de 3 pour chaque variable floue.
  			 > - DistToTarget: VeryClose, Close, Medium, Far, FarAway
			 > - AmmoStatus: NearlyOut, Low, Okay, Plenty, Loads
			 > - Desirability: Undersirable, SomewhatDesirable, Desirable, QuiteDesirable, VeryDesirable

  AddNoiseToAim:  
		    > Utilisation de la logique floue pour le calcul du bruit à ajouter à un tir.
		    > Variables floues utilisées:
		    > - DistToTarget (distance entre le bot et sa cible)
		    > - SpeedRatio (vitesse actuelle du bot par rapport à sa vitesse maximale)
		    > - TimeTargetVisible (durée pendant laquelle la cible du bot a été visible)


- Gameplay
< ... >



- Autre
  Weapon_GrenadeLauncher: 
  > Ajout d'une nouvelle arme: la grenade
  Utilisée principalement sur les groupes d'ennemis
  

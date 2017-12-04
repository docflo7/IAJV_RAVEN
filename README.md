# IAJV_RAVEN

# Equipe
Desrousseaux Florian (DESF28039508)

Duport François 

Edorh François (EDOF19059507)

Guison Vianney (GUIV30069402)

Stievenard Thomas (STIT31079507)

# Ajouts

- Réseau neuronal

            Nous avons ajouter un bot apprennant, utilisant la bibliothèque de réseau de neurones disponible à cette adresse : https://github.com/BobbyAnguelov/NeuralNetwork  
            Lorsqu'un bot neural est ajouté au jeu, il apprend automatiquement à partir de données de jeu enregistrée, pour lui permettre de décider quand tirer.  
            Les données de jeux sont créées lorsque l'on prend le contrôle d'un bot, et que l'option "Record Data" est cochée dans le menu "Neural Network"

- Logique floue

  Weapon_RocketLauncher:
  
			 5 intervalles au lieu de 3 pour chaque variable floue.
  			 - DistToTarget: VeryClose, Close, Medium, Far, FarAway
			 - AmmoStatus: NearlyOut, Low, Okay, Plenty, Loads
			 - Desirability: Undersirable, SomewhatDesirable, Desirable, QuiteDesirable, VeryDesirable

  AddNoiseToAim:  
  
		    Utilisation de la logique floue pour le calcul du bruit à ajouter à un tir.
		    Variables floues utilisées:
		    - DistToTarget (distance entre le bot et sa cible)
		    - SpeedRatio (vitesse actuelle du bot par rapport à sa vitesse maximale)
		    - TimeTargetVisible (durée pendant laquelle la cible du bot a été visible)


- Gameplay

AddPlayer
>Ajout d'un bot, qui sera directement possédé

ChangePlayerVelocity
>Est appelé si on controle le joueur avec le clavier.
Prend en compte les touches appuyés pour détérminer la direction du joueur

Dans les menus on peut choisir d'ajouter un joueur, de controler le joueur avec la souris ou le clavier
et on peut définir quel type de clavier nous utilisons (QWERTY ou AZERTY) pour changer les touches servant au déplacement

Dans les menus, si un joueur est présent sur le terrain, le bouton 'ADD Player' est grisé pour éviter 
d'ajouter deux fois un personnage-joueur (ce qui n'est pas possible dans le code)

Les touches allant de 1 à 6 permettent de changer d'armes, le pointeur de la souris définit où le personnage vise,
le clic gauche permet de tirer, le clic droit permet de changer de bot controlé

Si on est en mode souris, le clic droit permet de donner la position a atteindre, en restant appuyé sur 'Q' tout en cliquant on
définir plusieurs positions succéssives à atteindre

Si on est en mode clavier, ZQSD (en AZERTY) ou WASD en (QWERTY) permettent de donner la direction du personnage 



- Autre

  Weapon_GrenadeLauncher:
  
			 Ajout d'une nouvelle arme: la grenade.
  			 Utilisée principalement sur les groupes d'ennemis.
  

convert -verbose ^
	-size 512x512 ^
	gradient:#EBDCC8-#A8C8FF ^
	( +append RecipeCopier-Icon.png -resize 320x320 -geometry +0+50 ) ^
	-gravity north ^
	-compose src-over -composite ^
	-gravity south ^
	-pointsize 48 ^
	-fill #303030 ^
	-font "Bahnschrift" ^
	-draw "text 0,30 'RECIPE COPIER'" ^
	RecipeCopier-Logo.jpg

#ifndef DUINOTAGV2_CPP
#define DUINOTAGV2_CPP

#include <DuinoTagV2.h>
//lasercode
//header

//types, such as states are in main header file

/*
us = microsecond
milestag packets are constructed as below:

2400us header, 600us pause, then 1200us transmit for 1 and 600us transmit for 0,etc
shot packet consists of header+one bit for packet type (0 for shot packet), 7 more bits for player id, 2 bits for team id, then 4 bits for damage
message packet consists of one bit for packet type, 7 more bits for message ID, 8 more bits for data, then 8 more bits for EOF
*/

//laser gun reference/triggers
//cpp

//for now, I will assume that the packet receiving is handled by something else
//let's write some functions

/*

void Suit::setUpPacket(){
	Bitshift packet;
	packet = (playerID * 64) + (teamID * 16) + currentProfile.damage;
	#ifdef DEBUG
	Serial.println("String Packet: ");
	for(unsigned int i=0; i<13; i++){
		Serial.print(packet.grab(i));
		Serial.print(", ");
	}
	Serial.println("");
	#endif
	shotPacket[0] = 2400; //header
	shotPacket[1] = 600; //first space
	shotPacket[2] = 600; //first zero to say shot packet
	shotPacket[3] = 600; //secound space
	unsigned int bitcounter=11;
	for(unsigned int i=4; i<30&&bitcounter>=0; i++){
		if(!(i & 1)){ //alternating
			if(packet.grab(bitcounter)==false){
				shotPacket[i] = 600; //0
			}
			else{
				shotPacket[i] = 1200; //1
			}
			bitcounter--;
		}
		else{
			shotPacket[i]=600;
		}
	}
}

#if IR_SETUP==1
void Suit::waitForSetup(IRrecv * showMe){
	bool raw[61];
	bool check=false;
	decode_results resulters;
	while(!check){
		if(showMe->decode(&resulters)){
			unsigned int counter2=0;
			unsigned int i=0;
			while((resulters.rawbuf[i]<40||resulters.rawbuf[i]>60)&&i<resulters.rawlen){
				i++;
			}
			for(; i<resulters.rawlen&&counter2<62; i+=2){
				switch(decodePulse(resulters.rawbuf[i])){
					case 0:
					raw[counter2]=false;
					counter2++;
					break;
					case 1:
					raw[counter2] = true;
					counter2++;
					break;
					default:
					break;
				}
			}
			//validate checksum
			if(counter2 > 54){
				Bitshift boolCheck;
				boolCheck=0;
				unsigned int total=0;
				unsigned int u=0;
				for(unsigned int o=60; o>52; o--){
					boolCheck.flip(u,raw[o]);
					u++;
				}
				for(unsigned int o=0; o<53; o++){
					if(raw[o]){
						total++;
					}
				}
				if(boolCheck.store==total){
					check=true;
				}
				#ifdef DEBUG
				else{
					Serial.println("Checksums do not match");
					Serial.println(total);
					Serial.print(boolCheck.store);
					Serial.println("");
					for(unsigned int i=0; i<61; i++){
						Serial.print(raw[i]);
					}
					Serial.println("");
				}
				#endif
			}
			#ifdef DEBUG
			else{
				Serial.println("Non-setup Packet recieved");
				Serial.println(counter2);
			}
			#endif
			showMe->enableIRIn();
			showMe->resume();
		}
	}
	//transfer raw to values
	{
		Bitshift temp;
		unsigned int pointer=0;
		for(unsigned int i=1; i>=0; i--){
			temp.flip(i, raw[pointer]);
			pointer++;
		}
		Bitshift temp2;
		for(unsigned int i=6; i>=0; i--){
			temp2.flip(i, raw[pointer]);
			pointer++;
		}
		
		Bitshift temp3;
		
		for(unsigned int i=7; i>=0; i--){
			temp3.flip(i, raw[pointer]);
			pointer++;
		}
		gunValues.damage=temp3.store;
		
		for(unsigned int i=7; i>=0; i--){
			temp3.flip(i, raw[pointer]);
			pointer++;
		}
		gunValues.clipSize=temp3.store;
		for(unsigned int i=7; i>=0; i--){
			temp3.flip(i, raw[pointer]);
			pointer++;
		}
		gunValues.clipNum=temp3.store;
		for(unsigned int i=7; i>=0; i--){
			temp3.flip(i, raw[pointer]);
			pointer++;
		}
		startHealth=temp3.store;
		for(unsigned int i=7; i>=0; i--){
			temp3.flip(i, raw[pointer]);
			pointer++;
		}
		unsigned int randomInt=pointer;
		armor=temp3.store;
		Bitshift temp8;
		for(unsigned int i=3; i>=0; i--){
			temp8.flip(i, raw[pointer]);
			pointer++;
		}
		gunValues.reload=temp8.store;
		
		setup(temp.store, temp2.store, showMe);
		#ifdef VERBOSE_DEBUG
		
		Serial.print("Armor bits: ");
		for(unsigned int i=randomInt-8; i<randomInt; i++){
			Serial.print(raw[i]);
		}
		
		Serial.println("MASSIVE DEBUG");
		Serial.print("team:");
		Serial.println(teamID);
		Serial.print("Player: ");
		Serial.println(playerID);
		Serial.print("Clip: ");
		Serial.println(clipSize);
		Serial.print("ClipNum: ");
		Serial.println(clipNum);
		Serial.print("Armor: ");
		Serial.println(armor);
		Serial.println("");
		
		#endif
	}
	
}
#endif

void Suit::setup(myByte iTeamID, myByte iPlayerID){
	//set all values to defaults except for the modifiers
	//first set modifiers
	teamID = iTeamID;
	playerID = iPlayerID;
	//now set the defaults!
	currentProfile=gunValues;
	display.setup(milesHealth(startHealth), currentProfile.clipSize, armor, teamID);
	display.playIdle();
	setUpPacket();
	//IR
}

Suit::Suit(void){
	//YAYs
	startHealth = HEALTH; //100
	respawnTime = RESPAWN_TIME; //3 secs
	armor = ARMOR; //50
	respawns = false;
	numRespawns=0;
	isDead=true;
	currentHealth=0;
	currentArmor=0;
	//lasergun setup
	
	gunValues.clipNum = CLIP_NUM;
	gunValues.clipSize = CLIP_SIZE; 
	//defaults
	gunValues.damage=DAMAGE;
	gunValues.rpm=RPM; 
	gunValues.reload=RELOAD;
	currentDelay=0;
	tmpTime=0;
	currentAmmo=CLIP_NUM;
	rpmDelay=1000/(milesRPM(gunValues.rpm)/60);
	currentProfile = gunValues;
	currentClip = gunValues.clipSize;
	currentAmmo = gunValues.clipNum;
}

parsedPacket Suit::readPacket(packet packetYay){
	#ifdef DEBUG
	Serial.println("The packet is as follows: ");
	for(int i=7; i>=0; i--){
		Serial.print(packetYay.data1.grab(i));
		Serial.print(", ");
	}
	Serial.println("");
	Serial.println("Part 2: ");
	for(int i=7; i>=0; i--){
		Serial.print(packetYay.data2.grab(i));
		Serial.print(", ");
	}
	Serial.println("");
	#endif
	parsedPacket superYay;
	if(!packetYay.data1.grab(7)){ //check very first (last?) bit for message type
		//shot packet
		//add stats here later
		if(packetYay.data2.store >> 4 == teamID){ //checks to see if shot by friend or foe, and if friendlyfire is on
			//take damage, and if I make a stats machine record player Id
			#ifdef DEBUG
			Serial.println("teamID is the same as shooter teamid");
			#endif
#if FRIENDLY_FIRE == true
			packetYay.data2.flip(6,0);
			packetYay.data2.flip(7,0); //wipe the teamID so I can grab the damage
			superYay.amount= milesDamage(packetYay.data2.store/4); //grab the damage value, exclude the teamID, divide by 4 because damage isn't long enough to fill byte, and it leaves 2 zeros on the right side
			superYay.whatToDo=cShot;
#if USE_STATS == true
			stat.addValue(sHit, packetYay.data1.store-128);
#endif
#else
			superYay.whatToDo=cNull;
#endif
		}
		else{
			packetYay.data2.store = packetYay.data2.store << 2;
			superYay.amount = milesDamage(packetYay.data2.store>>4);
#ifdef DEBUG
			Serial.print("Thing! :");
			Serial.println(packetYay.data2.store>>4);
#endif
			superYay.whatToDo=cShot;
#if USE_STATS == true
			stat.addValue(sHit, packetYay.data1.store);
#endif
		}
		
	}
	else{
		//b/c of the leading one in packet type, all messages are 128+whatever
		#ifdef DEBUG
		Serial.println("Mesasage packet!");
		#endif
		superYay.whatToDo = packetYay.data1.store;
		superYay.amount = packetYay.data2.store;
		#ifdef DEBUG
		Serial.println(packetYay.data1.store);
		Serial.println(packetYay.data2.store);
		#endif
	}
	
	if(superYay.amount > 500){
		#ifdef DEBUG
		Serial.println("amount not initilized, so set to zero");
		#endif
		superYay.amount = 0;
	}
	return superYay;
	
}

bool Suit::action(packet packetYay){
	parsedPacket out;
	out = readPacket(packetYay);
	return sCommand(out.whatToDo, out.amount);
	
}

bool Suit::sCommand(unsigned int command, unsigned int amount = 0){
	//ugh, here we go
	//enumeration statments used to make decoding IR easier on me
	//don't have to look at numbers, and it's self documneting!
	if (command == cShot) {
		//play hit animation, subtract health, pause?
		if (!isDead) {
#if HIT_DELAY > 0
			delayMicroseconds(MHitDelay(HIT_DELAY)); //hit delay function turns hit delay myByte into seconds
#endif
			if (currentArmor <= 0) {
				currentHealth -= amount;
			}
			else {
				currentArmor -= amount;
				currentHealth -= amount / 2;
			}
#ifdef DEBUG
			Serial.print("Hit! Damage was: ");
			Serial.println(amount);
#endif
#ifdef VERBOSE_DEBUG
			Serial.print("Current Health is: ");
			Serial.println(currentHealth);
			Serial.print("And armor is: ");
			Serial.print(currentArmor);
			Serial.println("");
#endif
#if ON_HIT == true
			onHit();
#endif
			display.playLights(pLightsHit);
		}
#ifdef DEBUG
		else {
			Serial.println("Hit, but very dead!");
		}
#endif
	}
	else if (command == cSpecialCommand || command < 126) { //MT2Proto drives me nuts
		unsigned int switcher;
		if (command > 126) switcher = amount;
		else switcher = command;
		switch (switcher) {
		case cKill:
		{
			isDead = true;
#if USE_STATS == true
			stat.addValue(sDeath, 1);
#endif
			gunCommand(gStop, 0);
			display.playLights(pLightsDead);
			while (display.update()) {}
			display.playIdle();
			display.changeValues(0, 0, 0);
#if ON_DEATH == true
			onDeath();
#endif
		}
		break;
		case cPause:
		{
			//not implemented
		}
		break;
		case cStartGame:
		{
			if (!respawns) {
				//varibles! yay!
#if START_DELAY > 0
				delay(START_DELAY);
#endif
				respawns = RESPAWN_ALLOWED;
				currentHealth = milesHealth(startHealth);
				currentArmor = armor;
				isDead = false;
				gunCommand(gFullAmmo, 0);
				display.setup(milesHealth(startHealth), currentProfile.clipSize, armor, teamID);
				display.playLights(pLightsGameOn);
				while (display.update()) {}
				Serial.println("Got Here!");
#if ON_GAME_START == true
				onGameStart();
#endif
#ifdef DEBUG
				Serial.println("Game started!");
#endif
			}
		}
		break;
		case cDefaults:
		{
			setup(teamID, playerID);
			display.setup(milesHealth(startHealth), currentProfile.clipSize, armor, teamID);
			display.playLights(pLightsGameOn);
		}
		break;
		case cRespawn:
		{
#if RESPAWN_ALLOWED == true
			if (isDead) {
#if RESPAWN_LIMITED == true
				numRespawns++;
				if (numRespawns > MAX_RESPAWN) {
#ifdef DEBUG
					Serial.println("Respawn limit exceeded!");
#endif
					return;
				}
#endif
#ifdef DEBUG
				Serial.println("Respawn!");
#endif
#if RESPAWN_TIME > 0
				unsigned long int timeHold = millis();
				while (millis() - timeHold < respawnTime * 1000) checkStatus();
#endif
				isDead = false;
				currentHealth = milesHealth(startHealth);
				currentArmor = armor;
#if AMMO_RESTOCK == true
				gunCommand(gFullAmmo, 0);
#endif
				display.setup(milesHealth(startHealth), currentProfile.clipSize, armor, teamID);
#if ON_RESPAWN == true
				onRespawn();
#endif
				display.playLights(pLightsGameOn);
			}
#else
#ifdef DEBUG
			Serial.println("Respawns aren't allowed!");
#endif
#endif
		}
		break;
		case cINewGame:
		{
			respawns = RESPAWN_ALLOWED;
			currentHealth = milesHealth(startHealth);
			currentArmor = armor;
			isDead = false;
			gunCommand(gFullAmmo, 0);
			display.reset();
#if ON_GAME_START == true
			onGameStart();
#endif
			display.playLights(pLightsGameOn);
		}
		break;
		case cFullAmmo:
		{
			gunCommand(gFullAmmo, 0);
			display.playPew();
		}
		break;
		case cEndGame:
		{
			gunCommand(gStop, 0);
			isDead = true;
			respawns = false;
#if ON_GAME_END == true
			onGameEnd();
#endif
			display.playLights(pLightsGameOver);
			while (!display.update()) {}
			display.playIdle();
		}
		break;
		case cResetClock:
		{
			//will do later
		}
		break;
		case cExplodePlayer:
		{
			isDead = true;
#if USE_STATS == true
			stat.addValue(sDeath, 1);
#endif
			gunCommand(gStop, 0);
#if ON_DEATH == true
			onDeath();
#endif
			display.playLights(pLightsDead);
		}
		break;
		case cReadyUp:
		{
			//not implemented
		}
		break;
		case cFullHealth:
		{
			currentHealth = milesHealth(startHealth);
			display.playLights(pLightsGameOn);
		}
		break;
		case cFullArmor:
		{
			currentArmor = armor;
			display.playLights(pLightsGameOn);
		}
		break;
		case cClearScores:
		{
#if USE_STATS == true
			stat.reset();
			display.playLights(pLightsHit);
#endif
		}
		break;
		case cTestSensors:
		{
			//I have no idea how to do this yet, so whatever.
		}
		break;
		case cStun:
		{
			gunCommand(gStop, 0);
			display.playLights(pLightsHit);
			delay(2500);
		}
		break;
		case cDisarm:
		{
			gunCommand(gStop, 0);
			display.playLights(pLightsHit);
			delay(2500);
		}
		break;
		default:
		{
#ifdef DEBUG
			Serial.println("Unknown command recieved (1)");
#endif
			return false;
		}
		break;
		}
#if ON_CUSTOM_EVENT == true
		if (CUSTOM_EVENT == switcher) onCustomEvent();
#endif
	}
	else {
		switch (command) {
		case cAddHealth:
		{
			currentHealth += amount;
			//display.playLights(pLightsGameOn);
		}
		break;
		case cAddAmmo:
		{
			gunCommand(gAddAmmo, amount);
			//display.playLights(pLightsGameOn);
		}
		break;
		case cObj:
		{
#if ON_OBJECTIVE == true
			onObj(command);
#endif
		}
		break;
		default:
		{
#ifdef DEBUG
			Serial.println("Unknown command recieved (2)");
#endif
			return false;
		}
		break;
		}
#if ON_CUSTOM_EVENT == true
		if (CUSTOM_EVENT == command) onCustomEvent();
#endif
	}
	return true;
}

bool Suit::gunCommand(GunCommands command, unsigned int amount = 0){
	switch(command){
		case gStop:
		{
			display.reset();
			currentClip=0;
			return true;
		}
		break;
		case gShoot:
		{
			if(currentDelay>0){
				#ifdef VERBOSE_DEBUG
				Serial.println("Rpm delayed! Shot may not fire!");
				Serial.println((double)currentDelay);
				Serial.println("minus");
				Serial.println((millis()-tmpTime));
				#endif
				currentDelay=currentDelay - (millis()-tmpTime);
				tmpTime=millis();
			}
			if(currentClip>0&&!isDead&&currentDelay<=0){
				if(currentProfile.clipSize!=0xFF){
					currentClip--;
				}

				ir.shoot(shotPacket);

				#ifdef DEBUG
				Serial.println("BANG");
				Serial.println(rpmDelay);
				Serial.println(currentProfile.rpm);
				#endif
				currentDelay=rpmDelay;
#if USE_STATS == true
				stat.addValue(sShot,1);
#endif
				display.playPew();
#if ON_SHOOT == true
				onShoot();
#endif
				return true;
			}
			else{
				#ifdef DEBUG
				if(isDead){
					Serial.println("Shot not fired because death!");
				}
				else if(currentClip<=0){
					Serial.println("Shot not fired because currentClip<0");
				}
				else{
					Serial.println("Shot not fired because RPM delay!");
				}
				#endif
				if(currentDelay>0){
					return true;
				}
				return false;
			}
		}
		break;
		case gReload:
		{
			while(display.update() == true){
				display.changeValues(currentHealth,currentAmmo,currentArmor);
			}
			
			if(currentProfile.clipNum!=0xCA){ //check for unlimited
				currentAmmo--;
			}
			if(currentAmmo > 0&&!isDead){
#if ON_RELOAD_START == true
				onReloadStart();
#endif
				currentReload=currentProfile.reload*10;
				#ifdef DEBUG
				Serial.println("RELOADING!");
				Serial.print("Time: ");
				Serial.println(currentReload);
				Serial.println(currentProfile.reload);
				#endif
				unsigned long int lastTime=millis();
				unsigned int reloadStatus=0;
				while(currentReload>0){
					if(millis()-lastTime>100){
						lastTime=millis();
						currentReload--;
						#ifdef DEBUG
						Serial.println(currentReload);
						#endif
					}
					reloadStatus = currentProfile.clipSize-((double)(currentReload/10.0) * (100/currentProfile.reload))/(100/currentProfile.clipSize);
					display.changeValues(currentHealth,reloadStatus,currentArmor);
					checkStatus();
				}
				currentClip=currentProfile.clipSize;
#if ON_RELOAD_END == true
				onReloadEnd();
#endif
#if USE_STATS == true
				stat.addValue(sReload,1);
#endif
				return true;
			}
			else{
				return false;
			}
		}
		break;
		case gOverheat:
		{
			//not implemented
			return false;
		}
		break;
		case gAddAmmo:
		{
			currentAmmo+=amount;
			return true;
		}
		break;
		case gFullAmmo:
		{
			currentClip=currentProfile.clipSize;
			currentAmmo=currentProfile.clipNum;
			return true;
		}
		break;
		case gTest:
		{
			//no entiendo, por favor (me no spanish)
			//not implemented
			return false;
		}
		break;
	}
}

#if CUSTOM_WEAPONS == true
void Suit::switchGun(gunProfile newGun){
	currentProfile=newGun;
	currentClip = currentProfile.clipSize;
	#ifdef DEBUG
	Serial.println("Changing weapons!");
	Serial.println(currentClip);
	Serial.println(currentAmmo);
	#endif
	display.setup(milesHealth(startHealth), currentProfile.clipSize, armor, teamID);
	rpmDelay=1000/(milesRPM(currentProfile.rpm)/60);
	setUpPacket();
}
#endif

bool Suit::checkStatus() { //this function will return is the user is dead, but will also check to see if any packets are ready for processing, and if so, proccess them and take apropriete action
	packet hereWeGo;
	if(ir.checkRecieve(&hereWeGo)){
		if(!action(hereWeGo)){
			#ifdef DEBUG
			Serial.println("Parsing packet failed!");
			#endif
		}
		#ifdef DEBUG
		else{
			Serial.println("Parsing packet succeded!");
		}
		#endif
	}
	if(currentHealth<1 && !isDead){
#if DEATH_DELAY > 0
		long unsigned int timeHold = millis();
		while (millis() - timeHold > DEATH_DELAY * 1000) display.update();
#endif
		display.playLights(pLightsDead);
		display.changeValues(0,0,0);
		while(display.update()){}
		display.playIdle();
		delay(255);
#if USE_STATS == true
		stat.addValue(sDeath,1);
#endif
		isDead=true;
#if ON_DEATH == true
		onDeath();
#endif
	}
	if(currentClip>0){ //reload function will handle it
		display.changeValues(currentHealth,currentClip,currentArmor);
	}
	display.update();
	return isDead;
}

#if USE_STATS == true
//Stats machine
Stats::Stats(){
	reset();
}

void Stats::reset(){
	for(unsigned int i=0; i<127; i++){
		hitCount[i]=0;
	}
	deathCount=0;
	shotCount=0;
	reloadCount=0;
}

unsigned int Stats::calculate(statCommand command){
	switch(command){
		case sShot:
		return shotCount;
		break;
		case sDeath:
		return deathCount;
		break;
		case sReload:
		return reloadCount;
		break;
		default:
		return 0;
		break;
	}
}

void Stats::returnHits(unsigned int * ray){
	for(unsigned int i=0; i<127; i++){
		ray[i] = hitCount[i];
	}
}

void Stats::addValue(statCommand command, unsigned int input){
	switch(command){
		case sShot:
		shotCount+=input;
		break;
		case sDeath:
		deathCount+=input;
		break;
		case sReload:
		reloadCount+=input;
		break;
		case sHit:
		hitCount[input]++;
		break;
		default:
		break;
	}
}
#endif
*/

#endif //guard
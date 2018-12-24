//#pragma GCC push_options
//#pragma GCC optimize ("O0")

#include "print.h"

volatile char volnull = '\0';

// Private functions
//inline bool isFSFlag(uint8_t c);
//inline bool isFSWidth(uint8_t c);
//inline bool	isFSPrec(uint8_t c);
//inline bool isFSLength(uint8_t c);
//inline bool isFSSpec(uint8_t c);


//PRINT_Stat_e	printf_ll(PRINT_Intfc_t* pintfc, char_p format, va_list vl);
//int  snprintToIntfc(PRINT_Intfc_t* pintfc, char type, uint8_t numStars, char* f, va_list vl);

void handleRaw(PRINT_Intfc_t* pintfc, char_pp fp);		// Call when remaining data to transmit is not a format specifier
char handleForm(PRINT_Intfc_t* pintfc, char_pp fp, va_list vl);	// Call when remaining data starts with a format specifier
//void handleForm(PRINT_Intfc_t* pintfc, va_list vlp);







PRINT_Stat_e	mprintf(PRINT_Intfc_t* pintfc, const char* format, ...){
	PRINT_Stat_e stat = PRINT_ok;

	va_list vl;
	va_start(vl,format);
//	stat = printf_ll(pintfc, format, vl);

	int cx = vsnprintf((char*)pintfc->pbuff, (size_t)pintfc->bufflen, format, vl );
	if(cx >= 0){
		if( cx <= pintfc->bufflen){
			// If this case then no need to dynamically allocate - output is ready to be sent from the user's buffer
			stat = mprint(pintfc, (const char*)pintfc->pbuff);
		}
		else{
			// In this case the number of bytes required for the printf expansion is greater than that in the user's buffer.
#if PRINT_DYN
			// User allows dynamic memory allocation so let's try that (but only if cx is less than the allowed dynamic bytes
			if(cx <= PRINT_DYN_MAX){
				char* dynptr = NULL;
				if(pintfc->malloc != NULL){
					uint32_t dynamicSize = (((uint32_t)cx) + 1); // Add one for the null character
					dynptr = (char*)pintfc->malloc(dynamicSize*sizeof(char));
					if(dynptr != NULL){
						int dx = vsnprintf(dynptr, (size_t)dynamicSize, format, vl );
						if(dx >= 0){
							if( dx <= cx ){
								// Hooray! Then you can send the data
								stat = mprint(pintfc, (const char*)dynptr);
							}
							else{
								// This shouldn't really ever happen... it would require the format string to be changed during this function - and
								// because it is declare const it *should* not change
								stat = PRINT_error; // Error was that format was changed between the two vsnprintf calls
							}
						}
						else{
							stat = PRINT_error; // Error was an snprintf formatting error
						}
						free(dynptr); // Prevent dem leaks yo!
					}
					else{
						stat = PRINT_error; // Error was that malloc could not allocate the required number of bytes
					}
				}
				else{
					stat = PRINT_error; // Error was that pintfc->malloc was NULL
				}
			}
			else{
				stat = PRINT_error;	// Error was that cx (required expansion bytes) was greater than the maximum allowable dynamic allocation for the print library
			}
#else
			stat = PRINT_error;	// Error was that user's buffer was too small for expansion and they don't allow dynamic allocation
#endif
		}
	}
	else{
		stat = PRINT_error; // cx < 0 is a formatting error from vsnprintf
	}
	va_end(vl);

#if PRINT_ERR
	if(stat != PRINT_ok)
	{
//		printf_ll(pintfc, "PRINT_error", vl);
		mprint(pintfc, "PRINT_error");
	}
#endif
	return stat;
}

PRINT_Stat_e	mprint(PRINT_Intfc_t* pintfc, const char* pdata )
{
	PRINT_Stat_e	txStat = PRINT_ok;

	// Make sure it is OK to start
	if(pintfc == NULL){ return PRINT_error; }
	if(pintfc->pbuff == NULL){ return PRINT_error; }
	if(pintfc->bufflen == 0){ return PRINT_error; }
	if(pdata == NULL){ return PRINT_error; }

	bool done 		= false;
	const char*		f 	= pdata;			// Pointer to current part of string

	while(!done && (txStat == PRINT_ok)){

		// Handle the raw bytes
		handleRaw(pintfc, &f);

		// Now check to see if we are done...
		if(*(f) == '\0'){ done = true; }	// Done if the last byte put into the transmit buffer was null terminator

		// Transmit any bytes as needed before exiting or looping
		if(pintfc->btt > 0){
			// Transmit bytes
			while(pintfc->busy){}; 											// Wait for the hardware to be ready to transmit
			txStat = pintfc->txBytes(pintfc, pintfc->pbuff, pintfc->btt);	// Call the user's transmit function
			if(txStat != PRINT_ok){
				// ToDo: anything to do for a graceful bail?
				txStat = PRINT_error;
			}
			pintfc->btt = 0;
		}
	}

	return txStat;
}










//PRINT_Stat_e	printf_ll(PRINT_Intfc_t* pintfc, char* format, va_list vl)
//{
//	PRINT_Stat_e	txStat = PRINT_ok;
//
//	// Make sure it is OK to start
//	if(pintfc == NULL){ return PRINT_error; }
//	if(pintfc->pbuff == NULL){ return PRINT_error; }
//	if(pintfc->bufflen == 0){ return PRINT_error; }
//	if(format == NULL){ return PRINT_error; }
//
//	bool done 		= false;
//	bool specifier 	= false;
//	char*		f 	= format;			// Pointer to current part of format string
//
////	while(!done && (txStat == PRINT_ok)){
//	while(!done){
//
//		// Determine whether to handle raw bytes or a format specifier
//		if((*(f)) == '%'){
//			specifier = true;
//		}
//
//		// These functions leave f pointing at the next character, even if that character is out-of-bounds
//		// You will need to look back one character to make sure the last one was not the null terminator
//		if(specifier){
//			specifier = false;
////			handleForm(pintfc, &f, vl); // The old way
//
//			handleForm(pintfc, &f, vl);
//		}
//		else{
//			handleRaw(pintfc, &f);
//		}
//
//		// Now check to see if we are done...
//		if(*(f) == '\0'){ done = true; }	// Done if the last byte put into the transmit buffer was null terminator
//
//		// Transmit any bytes as needed before exiting or looping
//		if(pintfc->btt > 0){
//			// Transmit bytes
//			while(pintfc->busy){}; 											// Wait for the hardware to be ready to transmit
//			txStat = pintfc->txBytes(pintfc, pintfc->pbuff, pintfc->btt);	// Call the user's transmit function
//			if(txStat != PRINT_ok){
//				// ToDo: anything to do for a graceful bail?
//				txStat = PRINT_error;
//			}
//			pintfc->btt = 0;
//		}
//	}
//
//	return txStat;
//}













void handleRaw(PRINT_Intfc_t* pintfc, char** fp)
{
	// Put bytes from *(fp) into pintfc->pbuff until either:
	// a) the next byte leads a format specifier
	// b) the pintfc->pbuff is full
	// c) the next character is the null terminator
	// (p.s. f is to be incremented when a byte is placed into the interface buffer)
	uint8_t* b = pintfc->pbuff;
	bool done = false;

	if(pintfc->btt >= pintfc->bufflen){ done = true; }	// Just make sure that we're not already over limits
	if(*(*fp) == '\0'){ done = true; }					// Don't transmit the null terminator

	while(!done){
		*(b) = *(*fp); 	// Copy
		pintfc->btt++;	// Increment the number of bytes to transfer

		b++;			// Increment the pointers
		(*fp)++;

		if(pintfc->btt >= pintfc->bufflen){ done = true; } 	// If the transmit buffer is full then we need to stop for now
		if(*(*fp) == '\0'){ done = true; }					// If the next character is the null terminator then we are done
		if(*(*fp) == '%'){ done = true; }					// If the next character is the control character then we're done because we need to check what to do next
	}
}



////void handleForm(PRINT_Intfc_t* pintfc, char** fp, va_list vlp) // Made "optimize 0" to test a theory
////void handleForm(PRINT_Intfc_t* pintfc, va_list vlp)					// This form uses the "global" fp variable to try to undo some optimization
//char handleForm(PRINT_Intfc_t* pintfc, char** fp, va_list vlp)
//{
//	char theCompilerIsDumbAndOptimizedOutVeryImportantCode = 0xAA;
//
//	// Here *f is '%' so we suspect to find a format specifier
//	uint8_t starCount = 0;
//
//	uint32_t spec_bytes = 0;
//
//	char* fsp = *(fp);	// Save the head of the format specifier for later
//	(*(fp))++;			// Advance f
//	spec_bytes++;
//
//	bool earlyTerm = false;	// Indicate if we fail early
//	earlyTerm = (*(*(fp)) == '\0');
//
//	// Phase1: flags. Advance f past the flags field.
//	while(isFSFlag(*(*(fp))) && (!earlyTerm)){
//		(*(fp))++;
//		spec_bytes++;
//		if(*(*(fp)) == '\0'){ earlyTerm = true; }
//	}
//	// Phase2: width.
//	while(isFSWidth(*(*(fp))) && (!earlyTerm)){
//		if(*(*(fp)) == '*'){ starCount++; }	// This indicates that we will need an int argument for this dynamic field
//		(*(fp))++;
//		spec_bytes++;
//		if(*(*(fp)) == '\0'){ earlyTerm = true; }
//	}
//	// Phase3: precision.
//	if((*(*(fp)) == '.') && (!earlyTerm)){
//		(*(fp))++;
//		spec_bytes++;
//		if(*(*(fp)) == '\0'){ earlyTerm = true; }
//		while(isFSPrec(*(*(fp))) && (!earlyTerm)){
//			if(*(*(fp)) == '*'){ starCount++; }	// This indicates that we will need an int argument for this dynamic field
//			(*(fp))++;
//			spec_bytes++;
//			if(*(*(fp)) == '\0'){ earlyTerm = true; }
//		}
//	}
//	// Phase4: length.
//	while(isFSLength(*(*(fp))) && (!earlyTerm)){
//		(*(fp))++;
//		spec_bytes++;
//		if(*(*(fp)) == '\0'){ earlyTerm = true; }
//	}
//	// Phase5: specifier.
//	if(isFSSpec(*(*(fp))) && (!earlyTerm)){
//		(*(fp))++;
//		spec_bytes++;
//	}
//
//	if(earlyTerm){
//		// This means we ran into the null terminator before the whole expected format specifier sequence was read.
//		// ToDo: handle this error better
//		// For now:
//		// Because of how the upper function works if we return with *f == '\0' (and *(f-1) != '\0'))
//		// then the last null character will be sent by the normal text handler.
//		return;
//	}
//	else{
//		// If not terminated early then f points to the character right after the format specifier 'type' character.
//		// We will proceed to use snprintf to evaluate this one format specifier into the output buffer
//
//		char spec  = *(*(fp)-1);	// Extract the type of the format specifier so that we can properly cast our variable list
//		char jewel = *(*(fp));		// Take the jewel from the pedestal (save the value to be put back later)
////		*((char volatile*)*(fp)) = '\0';			// Set the char after the specifier to the null terminator so that snprintf doesn't touch our stuff
////		(*((volatile char*)*(fp))) = volnull; // Still trying to remove the optimization...
//		**fp = volnull; // FUCK YOU
//
//
//		// Alright, well we learned a thing. Strings created the way we would usually do it for printf are likely (but not guaranteed) to be
//		// stored in the executable (Read-only!) memory. So trying to set the value by pointer is futile - it can't be done.
//
//		// Use snprintf
//		theCompilerIsDumbAndOptimizedOutVeryImportantCode = *((char volatile*)*(fp));	// Use the value that we put at the end of the code to keep the compiler
//		// from optimizing out that line (I think it happens because we say *(*fp) = a and *(*fp) = b without any (obvious to compiler) use of fp in
//		// between. But really we are using it by knowing that it comes after fsp. But now we don't use that value... so will it be optimized out?
//
////		pintfc->trickChar = theCompilerIsDumbAndOptimizedOutVeryImportantCode;	// Try to trick the compiler into not optimizing fp!
//
////		signed int cx = (uint16_t)snprintToIntfc(pintfc, spec, starCount, fsp, vlp);
//		signed int cx = (uint16_t)snprintToIntfc(pintfc, spec, starCount, (*(fp) - spec_bytes), vlp);
//
//		// Decide if or if not to transmit...
//		if( cx > 0){
//			if( cx < pintfc->bufflen ){ pintfc->btt = (uint16_t)cx; }	// This will cause the upper function to try to send cx bytes.
//			// Otherwise btt is unchanged so these bytes will just disappear
//		}
//
////		*(*(fp)) = jewel;		// Put the jewel back like Indiana Jones (keeps text after format specifier ready-to-transmit)
//	// ToDo: the above line NEEDS to be uncommented in th elong run. Its just a test to see if this optimization thing really is the problem.
//
//
//	}
//
//	return theCompilerIsDumbAndOptimizedOutVeryImportantCode;
//}


//int  snprintToIntfc(PRINT_Intfc_t* pintfc, char type, uint8_t numStars, char* fsp, va_list vl)
//{
//	int dv1 = 0;
//	int dv2 = 0;
//
//	int cx = 0;
//	switch(type)
//	{
//		case 'd' :
//		case 'i' :
//			{
//				// Signed int
//				int arg = va_arg( vl, int );
//				switch(numStars)
//				{
//				case 0 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//					break;
//				case 1 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//					break;
//				case 2 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//					break;
//				default :
////					txStat = PRINT_error;
//					return -1;
//					break;
//				}
//			}
//			break;
//
//		case 'u' :
//		case 'x' :
//		case 'X' :
//		case 'o' :
//			{
//				// Unsigned int
//				unsigned arg = va_arg( vl, unsigned );
//				switch(numStars)
//				{
//				case 0 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//					break;
//				case 1 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//					break;
//				case 2 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//					break;
//				default :
////					txStat = PRINT_error;
//					return -1;
//					break;
//				}
//			}
//			break;
//
//		case 'f' :
//		case 'F' :
//		case 'e' :
//		case 'E' :
//		case 'g' :
//		case 'G' :
//		case 'a' :
//		case 'A' :
//			{
//				// Double
//				double arg = va_arg( vl, double );
//				switch(numStars)
//				{
//				case 0 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//					break;
//				case 1 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//					break;
//				case 2 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//					break;
//				default :
////					txStat = PRINT_error;
//					return -1;
//					break;
//				}
//			}
//			break;
//
//		case 'c' :
//			{
//				// Char
////				char arg = va_arg( vl, char ); //  warning: 'char' is promoted to 'int' when passed through '...'
//				signed arg = va_arg( vl, signed );
//				switch(numStars)
//				{
//				case 0 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//					break;
//				case 1 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//					break;
//				case 2 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//					break;
//				default :
////					txStat = PRINT_error;
//					return -1;
//					break;
//				}
//			}
//			break;
//
//		case 's' :
//			{
//				// Null terminated char array (C-string)
//				char* arg = (char*)va_arg( vl, print_sp_t );
//				switch(numStars)
//				{
//				case 0 :
//					cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//					break;
//
//					// Note: I'm fairly sure that the string type should not have any dynamic field values
//				case 1 :
//				case 2 :
//				default :
////					txStat = PRINT_error;
//					return -1;
//					break;
//				}
//			}
//			break;
//
//		case 'n' :
//		{
//			// Fills in the number of written chars so far into an integer pointer type
//			int* arg = (int*)va_arg( vl, print_ip_t );
//			switch(numStars)
//			{
//			case 0 :
//				cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//				break;
//
//				// Note: I'm fairly sure that the %n should not have any dynamic field values
//			case 1 :
//			case 2 :
//			default :
////				txStat = PRINT_error;
//				return -1;
//				break;
//			}
//		}
//		break;
//
//		case 'p' :
//		default :
////			txStat = PRINT_error;
//			return -1;
//			break;
//	}
//
//	return cx;
//}


//bool isFSFlag(uint8_t c){
//	switch(c)
//	{
//	case '-' :
//	case '+' :
//	case ' ' :
//	case '0' :
//	case '#' :
//		return true;
//		break;
//	default :
//		return false;
//		break;
//	}
//}
//bool isFSWidth(uint8_t c){
//	switch(c)
//	{
//	case '1' :
//	case '2' :
//	case '3' :
//	case '4' :
//	case '5' :
//	case '6' :
//	case '7' :
//	case '8' :
//	case '9' :
//	case '*' :
//		return true;
//		break;
//	case '0':
//	default :
//		return false;
//		break;
//	}
//}
//bool	isFSPrec(uint8_t c){
//	switch(c)
//	{
//	case '0':
//	case '1' :
//	case '2' :
//	case '3' :
//	case '4' :
//	case '5' :
//	case '6' :
//	case '7' :
//	case '8' :
//	case '9' :
//	case '*' :
//		return true;
//		break;
//	default :
//		return false;
//		break;
//	}
//}
//bool isFSLength(uint8_t c){
//	switch(c)
//	{
//	case 'h' :
//	case 'l' :
//	case 'L' :
//	case 'j' :
//	case 't' :
//		return true;
//		break;
//	default :
//		return false;
//		break;
//	}
//}
//bool isFSSpec(uint8_t c){
//	switch(c)
//	{
//	case 'd' :
//	case 'i' :
//	case 'u' :
//	case 'f' :
//	case 'F' :
//	case 'e' :
//	case 'E' :
//	case 'g' :
//	case 'G' :
//	case 'x' :
//	case 'X' :
//	case 'o' :
//	case 's' :
//	case 'c' :
//	case 'a' :
//	case 'A' :
//	case 'n' :
//	case '%' : // Added this to support escaping the escape character
//		return true;
//		break;
//	default :
//		return false;
//		break;
//	}
//}




/* Boneyard */

/*

//	// Check initial conditions
//	if(*(f) == '\0'){
//		complete = true; 		// We don't transmit the terminating character
//	}
//	if(*(f) == '%'){
//		if(*(f++) == '%'){
//			f++;
//		}
//		else{
//			pause = true;
//			specifier = true;
//		}
//	}
//
//
//	while(!complete){
//		while((!pause) && (!complete))
//		{
//			// When you're here you know that *(f) needs to be placed into the buffer
//			*(b++) = (uint8_t)(*(f++));
//			btt++;
//
//			// Read face-value bytes until the escape character '%' is found or until the txbuffer is filled up
//			if( *(f) == '\0' ){
//				complete = true;
//			}
//			if(btt == pintfc->bufflen){ 	// Send buffer is filled up, need to pause to send
//				pause = true;
//			}
//			if( *(f) == '%' ){				// Escape char found
//				if(*(f+1) == '%'){			// If followed by escape again then the intent is to print a '%'
//					f++;
//				}
//				else{
//					pause = true;			// If it is not followed by an escape then this is a format specifier so pause to handle it
//					specifier = true;
//				}
//			}
//		}
//
//		// When you're here and pause is true then you need to handle either a transmission or a format specifier
//		if(pause){
//			if(btt == pintfc->bufflen){
//				// Handle a transmit
//				while(pintfc->busy){}; 							// Wait for the hardware to be ready to transmit
//				txStat = pintfc->txBytes(pintfc, pintfc->pbuff, btt);	// Call the user's transmit function
//				if(txStat != PRINT_ok){
//					// ToDo: anything to do for a graceful bail?
//					txStat = PRINT_error;
//				}
//				btt = 0;
//				b 	= pintfc->pbuff;
//			}
//			if(specifier){
//				// Handle specifiers
//				specifier = false; // After we handle it specifier will no longer apply...
//
//				// We will need a variable to track how many '*' arguments are in the specifier (up to two)
//				uint8_t numStars = 0;
//
//				// Currently *(f) is the leading '%' and the next char is not '%'
//				char* fsp = f; // Save the pointer to the head of this format specifier pointer
//
//				// Now you can increment f
//				f++;
//				if(*(f) == '\0'){	// Make sure
//					complete = true;
//				}
//				else{
//					// For now I will just try to run past all the format specifier characters. Later we can handle them using snprintf()
//					// Edit: now I am going to handle the format specifiers
//
//					bool eos = false; // End of string... (in case we run into a null character)
//					eos = (*(f) == '\0');
//
//					// Phase1: flags.
//					while(isFSFlag(*(f)) && (!eos)){
//						// Run past them...
//						f++;
//						eos = (*(f) == '\0');
//					}
//					// Phase2: width.
//					while(isFSWidth(*(f)) && (!eos)){
//						// Run past them...
//						if(*(f) == '*'){
//							numStars++;	// This indicates that we will need an int argument for this dynamic field
//						}
//						f++;
//						eos = (*(f) == '\0');
//					}
//					// Phase3: precision.
//					if(*(f) == '.'){
//						f++;
//						eos = (*(f) == '\0');
//						while(isFSPrec(*(f)) && (!eos)){
//							// Run past them...
//							if(*(f) == '*'){
//								numStars++;	// This indicates that we will need an int argument for this dynamic field
//							}
//							f++;
//							eos = (*(f) == '\0');
//						}
//					}
//					// Phase4: length.
//					while(isFSLength(*(f)) && (!eos)){
//						// Run past them...
//						f++;
//						eos = (*(f) == '\0');
//					}
//					// Phase5: specifier.
//					if(isFSSpec(*(f))){
//						// Before we do anything else we need to know what variable type will be in the argument list at this point
//						uint8_t spec = *f; // Save the specifier
//
//						// Run past them...
//						f++;	// This is good. It leaves the format string pointer right where we will need it when we're done
//
//						// If the final character matches what we expected then we can hand this bit off to vsnprintf to expand into either:
//						// The user's buffer
//						// or
//						// A dedicated expansion buffer.
//
//						// Above we should add (todo) code to track the number of arguments that need to be passed. I.e. when a '*' is used in the specifier
//
//						// We also need to consider how to deal with the null-terminated output of vsnprintf and how to add null termination into the format 'snippet' that we hand to the function
//						// (so that it doesn't interfere with our job here)
//
//						// We need to terminate the format string with '\0' for vsnprintf
//						uint8_t jewel = (uint8_t)(*f); // Save the previous contents to replace later
////						*(f) = (char)'\0'; // Add the terminator so that we only process one
//						*f = (char)'\0'; // Add the terminator so that we only process one
//
//						// Now go ahead and process the format specifier using vsnprintf
//						int cx = 0;
//
//						if(numStars > 2){
//							txStat = PRINT_error;
//						}
//						else{
//							int dv1 = 0;	// Space for the first dynamic variable if it exists
//							int dv2 = 0;	// Space for the second dynamic variable if it exists
//
//							if(numStars > 0)
//							{
//								dv1 = va_arg( vl, int );
//							}
//
//							if(numStars > 1)
//							{
//								dv2 = va_arg( vl, int );
//							}
//
//							switch(spec)
//							{
//							case 'd' :
//							case 'i' :
//								{
//									// Signed int
//									int arg = va_arg( vl, int );
//									switch(numStars)
//									{
//									case 0 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//										break;
//									case 1 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//										break;
//									case 2 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//										break;
//									default :
//										txStat = PRINT_error;
//										break;
//									}
//								}
//								break;
//
//							case 'u' :
//							case 'x' :
//							case 'X' :
//							case 'o' :
//								{
//									// Unsigned int
//									unsigned arg = va_arg( vl, unsigned );
//									switch(numStars)
//									{
//									case 0 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//										break;
//									case 1 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//										break;
//									case 2 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//										break;
//									default :
//										txStat = PRINT_error;
//										break;
//									}
//								}
//								break;
//
//							case 'f' :
//							case 'F' :
//							case 'e' :
//							case 'E' :
//							case 'g' :
//							case 'G' :
//							case 'a' :
//							case 'A' :
//								{
//									// Double
//									double arg = va_arg( vl, double );
//									switch(numStars)
//									{
//									case 0 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//										break;
//									case 1 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//										break;
//									case 2 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//										break;
//									default :
//										txStat = PRINT_error;
//										break;
//									}
//								}
//								break;
//
//							case 'c' :
//								{
//									// Char
////									char arg = va_arg( vl, char ); //  warning: 'char' is promoted to 'int' when passed through '...'
//									signed arg = va_arg( vl, unsigned );
//									switch(numStars)
//									{
//									case 0 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//										break;
//									case 1 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
//										break;
//									case 2 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
//										break;
//									default :
//										txStat = PRINT_error;
//										break;
//									}
//								}
//								break;
//
//							case 's' :
//								{
//									// Null terminated char array (C-string)
//									char* arg = (char*)va_arg( vl, print_sp_t );
//									switch(numStars)
//									{
//									case 0 :
//										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//										break;
//
//										// Note: I'm fairly sure that the string type should not have any dynamic field values
//									case 1 :
////										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
////										break;
//									case 2 :
////										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
////										break;
//									default :
//										txStat = PRINT_error;
//										break;
//									}
//								}
//								break;
//
//							case 'n' :
//							{
//								// Fills in the number of written chars so far into an integer pointer type
//								int* arg = (int*)va_arg( vl, print_ip_t );
//								switch(numStars)
//								{
//								case 0 :
//									cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, arg);
//									break;
//
//									// Note: I'm fairly sure that the %n should not have any dynamic field values
//								case 1 :
////										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, arg);
////										break;
//								case 2 :
////										cx = snprintf( (char*)pintfc->pbuff, pintfc->bufflen, fsp, dv1, dv2, arg);
////										break;
//								default :
//									txStat = PRINT_error;
//									break;
//								}
//							}
//							break;
//
//							case 'p' :
//							default :
//								txStat = PRINT_error;
//								break;
//							}
//						}
//						// Okay now (hopefully) sprintf has filled something into our buffer.
//						if(txStat != PRINT_error){
//							if(cx > 0){
//
//								// Need to figure out how many bytes to transmit...
//								if(cx < pintfc->bufflen){
//									btt = cx;
//								}
//								else{
//									btt = pintfc->bufflen;
//									txStat = PRINT_error;
//								}
//
//								if(txStat != PRINT_error)
//								{
//									// Handle a transmit
//									while(pintfc->busy){}; 							// Wait for the hardware to be ready to transmit
//									txStat = pintfc->txBytes(pintfc, pintfc->pbuff, btt);	// Call the user's transmit function
//									if(txStat != PRINT_ok){
//										// ToDo: anything to do for a graceful bail?
//										txStat = PRINT_error;
//									}
//									btt = 0;
//									b 	= pintfc->pbuff;
//								}
//							}
//						}
//
//						// When done processing the format string put the jewel back like indiana jones
//						*(f) = jewel;
//					}
//					else{
//						// ToDo: anything to do for a graceful bail?
//						//return PRINT_error;
//						txStat = PRINT_error; // Error is that we expected a specifier here and didn't get one.
//					}
//				}
//			}
//		}
//
//		if(txStat != PRINT_ok){
//			// Bail out if an error has occurred (either transmit failed or a format specifier was wrong)
//			complete = true;
//		}
//
//		// Now continue as long as not complete
//		pause = false;
//	}
//
//	if(complete){
//		if(btt != 0){
//			while(pintfc->busy){}; 							// Wait for the hardware to be ready to transmit
//			txStat = pintfc->txBytes(pintfc, pintfc->pbuff, btt); 	// Finish up any transmission that might remain
//			if(txStat != PRINT_ok){
//				// ToDo: anything to do for a graceful bail?
//				txStat = PRINT_error;
//			}
//			btt = 0;
//			b 	= pintfc->pbuff;
//		}
//	}
//
////	va_end(vl);		// va_end(vl) is performed by the calling function
//	return txStat;

 */

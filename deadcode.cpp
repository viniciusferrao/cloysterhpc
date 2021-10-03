/* This is the graveyard where dead code resides for future reference */

#if 0
    tuiStartup();

    /* popen(); with "FILE*" to "fd" converter test to newt
     * this is barely working, the function is not async, so popen() is not
     * streaming to the TUI interface.
     * newt Windows aren't good either, they can't automatic scroll like tail
     * on a console, so the implementation is just bad.
     */
    FILE* file = popen("ls && sleep 3 && ls -l", "r");
    char *out = NULL;
    size_t outlen = 0;

    int fd = fileno(file);

    int width, height;
    char *flowedText = NULL;

    newtComponent form;
    newtComponent text;
    struct newtExitStruct es;
    newtCenteredWindow(74, 20, "Garbage Title");
    form = newtForm(NULL, NULL, 0);
    newtFormWatchFd(form, fd, NEWT_FD_READ);
    //newtFormSetTimer(form, 1000);
    //text = newtLabel(0, 0, out);
    //newtFormAddComponents(form, text, NULL);

    
	char *textBuffer = '\0';
    //char *textBuffer = NULL;
    //textBuffer = (char *) malloc(sizeof(char));
    while (getline(&out, &outlen, file) >= 0) {
        textBuffer = (char *) realloc(textBuffer, strlen(textBuffer) + strlen(out) + 1);
        strcat(textBuffer, out);
        //text = newtLabel(0, 0, textBuffer);
        system("sleep 0.1");
        //newtFormAddComponents(form, text, NULL);
		//newtFormRun(form, &es);
		if (es.reason == NEWT_EXIT_FDREADY) {
            //text = newtTextboxReflowed(1, 1, textBuffer, 70, 5, 5, NEWT_TEXTBOX_WRAP | NEWT_FLAG_SCROLL);
            
  
            flowedText = newtReflowText(textBuffer, 60, 5, 5, &width, &height);

            text = newtTextbox(1, 1, width, height, NEWT_TEXTBOX_WRAP | NEWT_TEXTBOX_SCROLL);
            newtTextboxSetText(text, flowedText);

            newtFormAddComponents(form, text, NULL);
			//printf("%s", textBuffer);
		} else if (es.reason == NEWT_EXIT_COMPONENT) {
            printf("WHAT\n");
		}
        newtFormRun(form, &es);
	}
    free(textBuffer);
    free(out);
	newtFormDestroy(form);

    tuiExit();
#endif

#if 0
void newtPipe(char *title, FILE* fd) {
    newtComponent form, text;
    char *out = NULL;
    size_t outlen = 0;

    newtCenteredWindow(74, 20, title);
    form = newtForm(NULL, NULL, 0);
    //newtFormWatchFd(form, fd, NEWT_FD_READ);
    //newtFormSetTimer(form, 100);

    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, fd);

    string[fsize] = 0;

    text = newtLabel(0, 0, string);
    newtFormAddComponents(form, text, NULL);
    newtFormWatchFd(form, (long) fd, NEWT_FD_READ);

    newtRunForm(form);

/*
    while (getline(&out, &outlen, fd) >= 0) {
        text = newtLabel(0, 0, out);
        newtFormAddComponents(form, text, NULL);
        newtFormWatchFd(form, (long) fd, NEWT_FD_READ);
        newtRunForm(form);
    }
*/
/*
    text = newtLabel(0, 0, message);
    newtFormWatchFd(form, fd, NEWT_FD_READ);
	newtFormSetTimer(form, 100);
    newtFormAddComponents(form, text, NULL);

    newtRunForm(form);
*/
	newtFormDestroy(form);


}
#endif

#if 0
    /* popen test with newt*/
    tuiStartup();

    FILE* file = popen("ls -l", "r"); // You should add error checking here.

    newtPipe(MSG_TITLE_HELP, file);

#if 0
    char *out = NULL;
    size_t outlen = 0;
    while (getline(&out, &outlen, file) >= 0)
    {
        //va_list args;
        //newtvwindow(MSG_TITLE_HELP, "PENE", out, args);
        newtPipe(MSG_TITLE_HELP, out, file);
        //system("sleep 0.1");
        //printf("%s", out);
        //printf("oi\n");
    }
#endif
    pclose(file);
//    free(out);

    tuiExit();
#endif

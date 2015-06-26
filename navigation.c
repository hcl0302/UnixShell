#include <menu.h>
#include <locale.h>
#include <panel.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define menu_max 50
#define filename_max 50

/*定义菜单组合结构体*/

typedef struct dirinfo  //包含一个菜单中的所有文件名、文件信息，和文件数
{
    char **filenames;
    char **info;
    int num;
}dirinfo;

typedef struct MenuGroup    //包含一个菜单的全部结构
{
    ITEM **items;
    MENU *menu;
    WINDOW *win;
    WINDOW *subwin;
    struct dirinfo dir;
}MenuGroup;

/*函数定义*/

void print_in_middle(WINDOW *win, int starty, int startx, int width,const char *string, chtype color);    //打印函数
void func(char *name);  //用户指针，备用

/*对文件操作的相关函数*/
int IsDir(char *path);   //判断是否时文件夹
void DirName(char *path,char *name); //根据路径得到当前文件夹名
int updateDir(char *path,int len);   //取得上层目录
dirinfo getFileNames(char *currentdir);  //返回当前目录下所有文件信息

/*创建一个菜单和用于装载它的窗体*/
MenuGroup CreateMenu(char *currentdir,int menu_num,const char *name);


/*函数实现*/

/*对文件操作的相关函数*/
int IsDir(char *path)   //判断是否时文件夹
{
    struct stat path_stat;
    if(!stat(path,&path_stat))
        if(S_ISDIR(path_stat.st_mode))   //路径是文件夹
            return 1;
    return 0;
}

void DirName(char *path,char *name) //根据路径得到当前文件夹名
{
    int len=strlen(path);
    if(len==1) strcpy(name,path);
    else {
        int i=len-1;
        for(;i>=0;i--)
            if(path[i]=='/')
                break;
        strcpy(name,&path[i+1]);
    }
}

int updateDir(char *path,int len)   //取得上层目录
{
    if(len==1) return 1;  //已到达根目录
    int i=len-1;
    for(;i>=0;i--)
        if(path[i]=='/')
            break;
    if(i==0) { path[1]=0; return 1;}
    path[i]=0;
    return i;
}

dirinfo getFileNames(char *currentdir)  //返回当前目录下所有文件信息
{
    dirinfo dir; dir.num=0;
    DIR *dp;
    struct dirent *dirp;
    struct stat path_stat;
    char path[1024];
    char **filenames,**info;
    int count=0,i,n;
    strcpy(path,currentdir);
    if((dp=opendir(path))==NULL)
        return dir;
    while((dirp=readdir(dp))!=NULL)  //第一遍先计数,方便分配空间
    {
        if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)  //跳过.和..这两个软链接
            continue;
        count++;
    }
    closedir(dp);
    if((dp=opendir(path))==NULL)  return dir;
    /*分配空间*/
    filenames=(char **)calloc(count+1,sizeof(char *));
    info=(char **)calloc(count+1,sizeof(char *));
    for(i=0;i<count+1;i++)
    {
        filenames[i]=(char *)malloc(filename_max*sizeof(char));
        info[i]=(char *)malloc(5*sizeof(char));
    }
    i=0; n=strlen(path);
    path[n++]='/'; path[n]=0;
    while((dirp=readdir(dp))!=NULL)  //遍历文件夹下的每一个文件或文件夹
    {
        if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)  //跳过.和..这两个软链接
            continue;
        strcpy(filenames[i],dirp->d_name);
        strcpy(&path[n],dirp->d_name);
        if(stat(path,&path_stat)!=0) continue;
        if(S_ISDIR(path_stat.st_mode))   //路径是文件夹
            strcpy(info[i],"dir");
        else strcpy(info[i],"file");
        i++;
        
    }
    closedir(dp); //关闭文件夹
    
    dir.filenames=filenames; dir.num=count; dir.info=info;
    return dir;
}

/*创建一个菜单和用于装载它的窗体*/

MenuGroup CreateMenu(char *currentdir,int menu_num,const char *name)
{
    MenuGroup newmenu;
    ITEM **my_items; 		
	MENU *my_menu;
    WINDOW *my_menu_win;
    dirinfo dir;
    char **filenames,**info;
    int n_choices,i;
	
     /*open the current work dir*/

    dir=getFileNames(currentdir);
    filenames=dir.filenames; info=dir.info; n_choices=dir.num;

	/* Create items */
        my_items = (ITEM **)calloc(n_choices+1, sizeof(ITEM *));
        //attron(COLOR_PAIR(1));
        int j=0;
        for(i = 0; i < n_choices; ++i) {
            if(info[i][0]=='d') //文件夹放在最前面
                my_items[j++] = new_item(filenames[i],info[i]);
                /*设置用户指针*/
                //set_item_userptr(my_items[i],func);
        }
        for(i = 0; i < n_choices; ++i) {
            if(info[i][0]!='d') //除了文件夹之外的
                my_items[j++] = new_item(filenames[i],info[i]);
                /*设置用户指针*/
                //set_item_userptr(my_items[i],func);
        }
        //attroff(COLOR_PAIR(1));
        my_items[n_choices]=(ITEM *)NULL;

	/* Crate menu */
	my_menu = new_menu((ITEM **)my_items);
	
    /* Create the window to be associated with the menu */
        my_menu_win = newwin(30, 40, 3, 1+41*menu_num);
        keypad(my_menu_win, TRUE);
     
	/* Set main window and sub window */
        set_menu_win(my_menu, my_menu_win);
        WINDOW *my_menu_win_sub=derwin(my_menu_win, 26, 38, 3, 1);
        set_menu_sub(my_menu, my_menu_win_sub);
        set_menu_format(my_menu,25,1);

	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");
	/* Print a border around the main window and print a title */
        box(my_menu_win, 0, 0);
        //set_menu_fore();
	print_in_middle(my_menu_win, 1, 0, 40,name, COLOR_PAIR(1));

	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
    move(1,15); clrtoeol();
    mvprintw(1,15,currentdir);
	refresh();
	/* Post the menu */
	post_menu(my_menu);
    //mvwchgat(my_menu_win_sub,5,0,-1,A_BOLD,1,NULL);
	wrefresh(my_menu_win);
    newmenu.items=my_items; newmenu.menu=my_menu;
    newmenu.win=my_menu_win; newmenu.subwin=my_menu_win_sub; 
    newmenu.dir=dir;
    return newmenu;
}

/*文件导航系统主控制函数*/

int Navigation(char *finalpath)
{
    MenuGroup menu_groups[menu_max];
    char currentdir[1024];
    PANEL *my_panels[menu_max];
    int panelhide=0;
    int menu_num=0;
	MENU *my_menu;
    WINDOW *my_menu_win;
    int i,c,items_num;

	
	/* Initialize curses */
    setlocale(LC_ALL,"");   //支持中文
	initscr();
	start_color();
    cbreak();
    noecho();
	keypad(stdscr, TRUE);
    //初始化颜色
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2,COLOR_BLUE, COLOR_BLACK);
    init_pair(3,COLOR_CYAN, COLOR_BLACK);
    init_pair(4,COLOR_MAGENTA, COLOR_BLACK);

    /*打印快捷键信息*/
    char keyinfo[14][25]={
        "Enter:"," save&exit  ","UP:"," previous item  ","DOWN:"," next item  ","PGUP:"," previous page  ","PGDN"," next page  ","LEFT:"," previous dirrectory  ","RIGHT:"," next dirrectory  "
    };
    move(LINES-4,2);
    for(i=0;i<14;i++) {
        attron(COLOR_PAIR(2));
        printw(keyinfo[i++]);
        attroff(COLOR_PAIR(2));
        printw(keyinfo[i]);
    }
    attron(COLOR_PAIR(4));
    mvprintw(LINES-6,2,"输入部分文件名并按上下键可快速查找匹配文件,按BackSpace可以清除前一个输入的字符");
    attroff(COLOR_PAIR(4));
    attron(COLOR_PAIR(3));
    mvprintw(1,1,"Current Path: ");
    attroff(COLOR_PAIR(3));

    getcwd(currentdir,sizeof(currentdir));
    char dname[30];
    DirName(currentdir,dname);
    int dir_len=strlen(currentdir);
    menu_groups[menu_num]=CreateMenu(currentdir,menu_num,(const char *)dname);

    my_menu=menu_groups[menu_num].menu;
    my_menu_win=menu_groups[menu_num].win;
    items_num=menu_groups[menu_num].dir.num;
    my_panels[menu_num]=new_panel(my_menu_win);
    update_panels();    //更新栈的顺序
    doupdate(); //在屏幕上显示

	while((c = wgetch(my_menu_win)) != 10)
	{       switch(c)
	        {/*	
            case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;*/
            case KEY_NPAGE:
				menu_driver(my_menu, REQ_SCR_DPAGE);
				break;
			case KEY_PPAGE:
				menu_driver(my_menu, REQ_SCR_UPAGE);
				break;
            case KEY_UP:
                menu_driver(my_menu, REQ_PREV_MATCH);
                break;
            case KEY_DOWN:
                menu_driver(my_menu, REQ_NEXT_MATCH);
                break;
            case KEY_BACKSPACE:
                menu_driver(my_menu, REQ_BACK_PATTERN);
                break;
            //case 10: //Enter键
            case KEY_RIGHT:
                {
                    if(items_num) {
                        ITEM *item;
                        item=current_item(my_menu);
                        if(item) {
                            const char *name=item_name(item);
                            strcpy(dname,name);
                            //pos_menu_cursor(my_menu);     定位光标
                            currentdir[dir_len]='/';
                            strcpy(&currentdir[dir_len+1],dname);
                            if(IsDir(currentdir)) {
                                dir_len=strlen(currentdir);
                                menu_num++;
                                menu_groups[menu_num]=CreateMenu(currentdir,menu_num,dname);
                                my_menu=menu_groups[menu_num].menu;
                                my_menu_win=menu_groups[menu_num].win;
                                items_num=menu_groups[menu_num].dir.num;
                                my_panels[menu_num]=new_panel(my_menu_win);
                                if(menu_num-panelhide>2)
                                {   //隐藏最左边的窗口并左移所有窗口
                                    hide_panel(my_panels[panelhide++]);
                                    move_panel(my_panels[panelhide],3,1);
                                    move_panel(my_panels[panelhide+1],3,42);
                                    move_panel(my_panels[panelhide+2],3,83);
                                }
                                update_panels();    //更新栈的顺序
                                doupdate(); //在屏幕上显示
                            }
                        }
                    }
                    break;
                }
            case KEY_LEFT:
                /*释放空间,清理*/
                unpost_menu(my_menu);
                free_menu(my_menu);
                for(i = 0; i <= items_num; i++)
                {
                    free_item(menu_groups[menu_num].items[i]);
                    free(menu_groups[menu_num].dir.filenames[i]);
                    free(menu_groups[menu_num].dir.info[i]);
                }
                free(menu_groups[menu_num].items);
                free(menu_groups[menu_num].dir.filenames);
                free(menu_groups[menu_num].dir.info);
                del_panel(my_panels[menu_num]);
                delwin(my_menu_win); delwin(menu_groups[menu_num].subwin);
                
                /*更新到上一层目录*/
                dir_len=updateDir(currentdir,dir_len);
                
                /*显示上一层目录*/
                if(menu_num)
                {
                    menu_num--;
                    if(panelhide) {
                        show_panel(my_panels[--panelhide]);
                        move_panel(my_panels[panelhide+1],3,42);
                        move_panel(my_panels[panelhide+2],3,83);
                    }
                    move(1,15); clrtoeol();
                    mvprintw(1,15,currentdir);
                }
                else {  //已经到了最底层
                    DirName(currentdir,dname);
                    menu_groups[menu_num]=CreateMenu(currentdir,menu_num,dname);
                    my_panels[menu_num]=new_panel(menu_groups[menu_num].win);
                }
                my_menu=menu_groups[menu_num].menu;
                my_menu_win=menu_groups[menu_num].win;
                items_num=menu_groups[menu_num].dir.num;
                update_panels();    //更新栈的顺序
                doupdate(); //在屏幕上显示
                break;
            default:
                menu_driver(my_menu,c);
                break;
		}
                wrefresh(my_menu_win);
	}	

	/* Unpost and free all the memory taken up */
    int j;
    for(i=menu_num;i>=0;i--) 
    {    
        unpost_menu(menu_groups[i].menu);
        free_menu(menu_groups[i].menu);
        for(j = 0; j <= menu_groups[i].dir.num; j++)
        {
            free_item(menu_groups[i].items[j]);
            free(menu_groups[i].dir.filenames[j]);
            free(menu_groups[i].dir.info[j]);
        }
        free(menu_groups[i].dir.filenames);
        free(menu_groups[i].dir.info);
        free(menu_groups[i].items);
    }
	endwin();
    strcpy(finalpath,currentdir);
    return 0;
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, const char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

void func(char *name)
{
    //move(35,0);
    //clrtoeol();
    mvprintw(35,0,"Item selected is: %s",name);
}

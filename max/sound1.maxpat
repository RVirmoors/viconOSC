{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 7,
			"minor" : 2,
			"revision" : 4,
			"architecture" : "x86",
			"modernui" : 1
		}
,
		"rect" : [ 67.0, 109.0, 640.0, 480.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-10",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 322.5, 89.0, 125.0, 20.0 ],
					"style" : "",
					"text" : "gesture trigger [1...5]\n"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 132.0, 89.0, 54.0, 20.0 ],
					"style" : "",
					"text" : "params\n"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 394.0, 377.0, 150.0, 20.0 ],
					"style" : "",
					"text" : "audio out\n"
				}

			}
, 			{
				"box" : 				{
					"comment" : "audio R",
					"id" : "obj-4",
					"maxclass" : "outlet",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 353.0, 372.0, 30.0, 30.0 ],
					"presentation_rect" : [ 353.0, 370.0, 0.0, 0.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"comment" : "audio L",
					"id" : "obj-3",
					"maxclass" : "outlet",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 144.0, 372.0, 30.0, 30.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"comment" : "gesture trigger",
					"id" : "obj-2",
					"maxclass" : "inlet",
					"numinlets" : 0,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 353.0, 123.0, 30.0, 30.0 ],
					"presentation_rect" : [ 353.0, 121.0, 0.0, 0.0 ],
					"style" : ""
				}

			}
, 			{
				"box" : 				{
					"comment" : "list of params",
					"id" : "obj-1",
					"maxclass" : "inlet",
					"numinlets" : 0,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 144.0, 123.0, 30.0, 30.0 ],
					"style" : ""
				}

			}
 ],
		"lines" : [  ],
		"dependency_cache" : [  ],
		"autosave" : 0
	}

}

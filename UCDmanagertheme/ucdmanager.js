(function ($) {
	// add 'edit' link in views
	Drupal.behaviors.vieweditlinks = {
		attach: function(context, settings) {
			$edit_class = '.views-field-edit-node';
			// hide view 'edit' links until hover
			$($edit_class, context).hide();
			// show edit link when hover
			$(".views-row, td.views-field-title, .view-content .item-list > ul > li", context).hover(
				function(){
					$(this).find($edit_class).first().show();
				},
				function(){
					$(this).find($edit_class).first().hide();
				}
			);
		}
	}

	// store last selected tab
	// (override call in module panel tabs, which doesn't includes cookie option)
	Drupal.behaviors.panelsTabs = {
	  attach: function (context) {
		if ( Drupal.settings.panelsTabs ) {
			var tabsID = Drupal.settings.panelsTabs.tabsID;
			for (var key in Drupal.settings.panelsTabs.tabsID) {
			  // remove existing widget?
			  $('#' + tabsID[key], context)
				.tabs({cookie:{expires:1}});
			} 
		}
	  }
	};

})(jQuery);
<!DOCTYPE html>
<!--[if IEMobile 7]><html class="iem7" <?php print $html_attributes; ?>><![endif]-->
<!--[if (lte IE 6)&(!IEMobile)]><html class="ie6 ie6-7 ie6-8" <?php print $html_attributes; ?>><![endif]-->
<!--[if (IE 7)&(!IEMobile)]><html class="ie7 ie6-7 ie6-8" <?php print $html_attributes; ?>><![endif]-->
<!--[if (IE 8)&(!IEMobile)]><html class="ie8 ie6-8" <?php print $html_attributes; ?>><![endif]-->
<!--[if (gte IE 9)|(gt IEMobile 7)]><!--><html <?php print $html_attributes . $rdf_namespaces; ?>><!--<![endif]-->
<head>
<?php print $head; ?>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="MobileOptimized" content="width">
<meta name="HandheldFriendly" content="true">
<meta name="apple-mobile-web-app-capable" content="yes">
<!-- Windows 8 app http://msdn.microsoft.com/en-us/library/windows/apps/hh781489.aspx -->
<meta name="msApplication-ID"content="df2605bb-7046-4d21-858e-c83822f80694"/>
<meta name="msApplication-PackageFamilyName"content="12084jordisan.UCDUser-CenteredDesign_szkqbfjhrv8ty"/>
<!--[if IEMobile]>  <meta http-equiv="cleartype" content="on">  <![endif]-->
<!-- meta http-equiv="X-UA-Compatible" content="IE=edge, chrome=1" -->
<title><?php print $head_title; ?></title>
<?php print $styles; ?>
<?php print $scripts; ?>
<!--[if lt IE 9]>
<script src="//html5shiv.googlecode.com/svn/trunk/html5.js"></script>
<![endif]-->
</head>
<body class="<?php print $classes; ?>"<?php print $attributes; ?>>
<?php if ( ! UCDmanagertheme_downloadFormat() ) : ?>
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>
<?php endif; ?>
  <?php print $page_top; ?>
  <?php print $page; ?>
  <?php print $page_bottom; ?>
</body>
</html>

<?php
	$is_teaser = ($view_mode == 'teaser');
	if ( $teaser ) {
		$title_tag = 'h3';
	} else {
		$title_tag = 'h1';
	}
?>
<article id="article-<?php print $node->nid; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>

  <?php print $unpublished; ?>

  <?php print render($title_prefix); ?>
  <?php if ($title && !$page): ?>
    <header>
      <?php if ($title): ?>
        <<?php print $title_tag ?><?php print $title_attributes; ?>>
          <a href="<?php print $node_url; ?>" rel="bookmark"><?php print $title; ?></a>
        </<?php print $title_tag ?>>
      <?php endif; ?>
    </header>
  <?php endif; ?>
  <?php print render($title_suffix); ?>

  <?php if ($display_submitted): ?>
    <footer class="submitted<?php $user_picture ? print ' with-user-picture' : ''; ?>">
      <?php // print $user_picture; ?>
      <p class="author-datetime"><?php print $submitted; ?></p>
	  <p class="author-datetime"><?php print $last_edit; ?></p>
    </footer>
  <?php endif; ?>

  <div<?php print $content_attributes; ?>>
  <?php
    hide($content['comments']);
    hide($content['links']);
    print render($content);
  ?>
  </div>

  <?php if ($links = render($content['links'])): ?>
    <nav class="clearfix"><?php print $links; ?></nav>
  <?php endif; ?>

  <?php // print render($content['comments']); ?>

</article>

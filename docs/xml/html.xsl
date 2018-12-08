<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">

<xsl:output method="xml"
	doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
	doctype-system="DTD/xhtml1-transitional.dtd"
	indent="no"/>

<!-- Keys -->
<xsl:key name="sub_ref" match="sub" use="@name"/>
<xsl:key name="section_ref" match="section" use="@title"/>


<!-- Templates -->
<xsl:template name="TOC">
	<xsl:for-each select="section">
		<p>
		<a><xsl:attribute name="href">#<xsl:value-of select="@title"/></xsl:attribute>
			<xsl:number level="multiple"
						count="section"
						format="1. "/>
				<xsl:value-of select="@title"/>
		</a>
		<br/>
		<xsl:for-each select="sub">
			   <a><xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
				<xsl:number level="multiple"
							count="section|sub"
							format="1."
							value="count(ancestor::section/preceding-sibling::section)+1"/>
				<xsl:number format="1. "/>
				<xsl:apply-templates select="header"/>
			</a>
			<br/>
		</xsl:for-each>
		</p>
	</xsl:for-each>
</xsl:template>


<xsl:template name="NAVBAR">
	<tr>
		<td align="center">
			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr>
					<td align="center"><img src="images/exult_logo.gif" width="181" height="127" alt="Exult Logo"/></td>
				</tr>
				<tr>
					<td><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
				</tr>
				<tr>
					<td><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
				</tr>
			</table>
		</td>
	</tr>
</xsl:template>


<xsl:template name="DISCLAIMER">
	<tr>
		<td><br/>
			<hr/>
		</td>
	</tr>
	<tr>
		<td>
			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr>
					<td align="center"><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
				</tr>
				<tr>
					<td align="center"><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
				</tr>
				<tr>
					<td align="center">
						<address><font size="-1">Problems with Exult or this webpage? <a href="http://exult.sourceforge.net/forum/post.php?f=1">Contact us.</a></font></address>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td align="center">
			<address><font size="-1">Last modified: <xsl:value-of select="@changed"/></font></address>
		</td>
	</tr>
	<tr>
		<td align="center">
			<div align="center">
				<xsl:comment>#exec cgi="cgi-bin/vipcounter_xml.pl"</xsl:comment></div>
		</td>
	</tr>
</xsl:template>

<!-- FAQ Template -->
<xsl:template match="faqs">
	<html>
	<xsl:comment>This file is automatically generated. Do not edit!</xsl:comment>
	<head>
		<title><xsl:value-of select="@title"/> FAQ</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	</head>
	<body bgcolor="#cccccc" text="#333366" link="#666699" vlink="#669966" alink="#ffcc33"  background="images/back.gif">
			<table border="0" cellpadding="0" cellspacing="0" width="90%">
				<xsl:call-template name="NAVBAR"/>
				<tr>
					<td>
						<h1><xsl:value-of select="@title"/> F.A.Q. (frequently asked questions) </h1>
						<p>last changed: <xsl:value-of select="@changed"/></p>
						<hr/>
						<p>
							The latest version of this document can be found <a href="http://exult.sourceforge.net/faq.php">here</a>
						</p>
						<br/>

					<!-- BEGIN TOC -->
					<xsl:call-template name="TOC"/>
					<!-- END TOC -->

					<!-- BEGIN CONTENT -->
					<xsl:apply-templates select="section"/>
					<!-- END CONTENT -->

					</td>
				</tr>
				<xsl:call-template name="DISCLAIMER"/>
			</table>
	</body>
	</html>
</xsl:template>

<!-- Readme Template -->
<xsl:template match="readme">
	<html>
	<xsl:comment>This file is automatically generated. Do not edit!</xsl:comment>
	<head>
		<title><xsl:value-of select="@title"/> - Documentation</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	</head>
	<body bgcolor="#cccccc" text="#333366" link="#666699" vlink="#669966" alink="#ffcc33" background="images/back.gif">
		<div align="center">
			<table border="0" cellpadding="0" cellspacing="0" width="90%">
				<xsl:call-template name="NAVBAR"/>
				<tr>
					<td>
						<h1><xsl:value-of select="@title"/> - Documentation </h1>
						<p>last changed: <xsl:value-of select="@changed"/></p>
						<hr/>
						<p>
							The latest version of this document can be found <a href="http://exult.sourceforge.net/docs.php">here</a>
						</p>
						<br/>

					<!-- BEGIN TOC -->
					<xsl:call-template name="TOC"/>
					<!-- END TOC -->

					<!-- BEGIN CONTENT -->
					<xsl:apply-templates select="section"/>
					<!-- END CONTENT -->

					</td>
				</tr>
				<xsl:call-template name="DISCLAIMER"/>
			</table>
		</div>
	</body>
	</html>
</xsl:template>

<!-- Studio Docs Template -->
<xsl:template match="studiodoc">
	<html>
	<xsl:comment>This file is automatically generated. Do not edit!</xsl:comment>
	<head>
		<title><xsl:value-of select="@title"/> Studio Documentation </title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	</head>
	<body bgcolor="#cccccc" text="#333366" link="#666699" vlink="#669966" alink="#ffcc33"  background="images/back.gif">
		<div align="center">
			<table border="0" cellpadding="0" cellspacing="0" width="90%">
				<xsl:call-template name="NAVBAR"/>
				<tr>
					<td>
						<h1><xsl:value-of select="@title"/> Studio Documentation </h1>
						<p>last changed: <xsl:value-of select="@changed"/></p>
						<hr/>
						<p>
							The latest version of this document can be found <a href="http://exult.sourceforge.net/studio.php">here</a>
						</p>
						<br/>

					<!-- BEGIN TOC -->
					<xsl:call-template name="TOC"/>
					<!-- END TOC -->

					<!-- BEGIN CONTENT -->
					<xsl:apply-templates select="section"/>
					<!-- END CONTENT -->

					</td>
				</tr>
				<xsl:call-template name="DISCLAIMER"/>
			</table>
		</div>
	</body>
	</html>
</xsl:template>

<!-- Group Template -->
<xsl:template match="section">
	<hr width="100%"/>
	<table width="100%">
		<tr><th align="left">
			<a><xsl:attribute name="name"><xsl:value-of select="@title"/></xsl:attribute>
				<xsl:number format="1. "/>
				<xsl:value-of select="@title"/>
			</a>
		</th></tr>
		<xsl:apply-templates select="sub"/>
	</table>
</xsl:template>


<!-- Entry Template -->
<xsl:template match="sub">
	<xsl:variable name = "num_idx">
		<xsl:number level="single"
					count="section"
					format="1."
					value="count(ancestor::section/preceding-sibling::section)+1"/>
		<xsl:number format="1. "/>
	</xsl:variable>
	<tr><td><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td></tr>
	<tr><td><strong>
		<a><xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
			<xsl:value-of select="$num_idx"/>
			<xsl:apply-templates select="header"/>
		</a>
	</strong></td></tr>
	<tr><td><xsl:apply-templates select="body"/></td></tr>
</xsl:template>


<xsl:template match="header">
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="body">
	<xsl:apply-templates/>
</xsl:template>


<!-- Internal Link Templates -->
<xsl:template match="ref">
	<a><xsl:attribute name="href">#<xsl:value-of select="@target"/></xsl:attribute>
		<xsl:value-of select="count(key('sub_ref',@target)/parent::section/preceding-sibling::section)+1"/>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="count(key('sub_ref',@target)/preceding-sibling::sub)+1"/>
		<xsl:text>.</xsl:text>
	</a>
</xsl:template>


<xsl:template match="ref1">
	<a><xsl:attribute name="href">#<xsl:value-of select="@target"/></xsl:attribute>
		<xsl:value-of select="count(key('sub_ref',@target)/parent::section/preceding-sibling::section)+1"/>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="count(key('sub_ref',@target)/preceding-sibling::sub)+1"/>
		<xsl:text>. </xsl:text>
		<xsl:apply-templates select="key('sub_ref',@target)/child::header"/>
	</a>
</xsl:template>


<xsl:template match="section_ref">
	<a><xsl:attribute name="href">#<xsl:value-of select="@target"/></xsl:attribute>
		<xsl:value-of select="count(key('section_ref',@target)/preceding-sibling::section)+1"/>
		<xsl:text>. </xsl:text>
  		<xsl:apply-templates select="key('section_ref',@target)/@title"/>
	</a>
</xsl:template>

<!-- External Link Template -->
<xsl:template match="extref">
	<a>
	<xsl:attribute name="href">
		<xsl:choose>
			<xsl:when test="@doc='faq'">
				<xsl:text>faq.html#</xsl:text>
			</xsl:when>
			<xsl:when test="@doc='docs'">
				<xsl:text>ReadMe.html#</xsl:text>
			</xsl:when>
			<xsl:when test="@doc='studio'">
				<xsl:text>exult_studio.html#</xsl:text>
			</xsl:when>
		</xsl:choose>
		<xsl:value-of select="@target"/>
	</xsl:attribute>
	<xsl:choose>
		<xsl:when test="count(child::node())>0">
				<xsl:value-of select="."/>
		</xsl:when>
		<xsl:when test="@doc='faq'">
			<xsl:text>FAQ</xsl:text>
		</xsl:when>
		<xsl:when test="@doc='docs'">
			<xsl:text>ReadMe</xsl:text>
		</xsl:when>
		<xsl:when test="@doc='studio'">
			<xsl:text>Studio Documentation</xsl:text>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="@target"/>
		</xsl:otherwise>
	</xsl:choose>
	</a>
</xsl:template>

<!-- Image Link Template -->
<xsl:template match="img">
	<xsl:copy>
		<xsl:for-each select="@*|node()">
	<xsl:copy/>
	</xsl:for-each>
	</xsl:copy>	
</xsl:template>

<!-- Misc Templates -->
<xsl:template match="Exult">
	<em>Exult</em>
</xsl:template>

<xsl:template match="Studio">
	<em>Exult Studio</em>
</xsl:template>

<xsl:template match="Pentagram">
	<em>Pentagram</em>
</xsl:template>

<xsl:template match="cite">
                <p>
                <xsl:value-of select="@name"/>:<br/>
                <cite><xsl:apply-templates/></cite>
                </p>
</xsl:template>

<xsl:template match="para">
	<p><xsl:apply-templates/></p>
</xsl:template>


<xsl:template match="key">
	'<font color="maroon"><xsl:value-of select="."/></font>'
</xsl:template>


<xsl:template match="kbd">
	<font color="maroon"><kbd><xsl:apply-templates/></kbd></font>
</xsl:template>

<xsl:template match="TM">
	<xsl:text disable-output-escaping="yes">&amp;trade;&amp;nbsp;</xsl:text>
</xsl:template>


<!-- ...................ol|dl|ul + em............... -->
<xsl:template match="ul|ol|li|strong|q|br">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="em">
 <b><i><font size="+1">
<xsl:apply-templates/>
</font></i></b>
</xsl:template>

<!-- Key Command Templates -->
<xsl:template match="keytable">
	<table border="0" cellpadding="0" cellspacing="2" width="80%">
		<tr>
			<th colspan="3" align="left">
				<xsl:value-of select="@title"/>
			</th>
		</tr>
		<xsl:apply-templates select="keydesc"/>
	</table>
</xsl:template>


<xsl:template match="keydesc">
	<tr>
		<td nowrap="nowrap" valign="top">
			<font color="maroon"><xsl:value-of select="@name"/></font>
		</td>
		<td width="10"><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text></td>
		<td><xsl:value-of select="."/></td>
	</tr>
</xsl:template>


<!-- Config Table Templates -->

<xsl:template match="configdesc">
	<table border="0" cellpadding="0" cellspacing="0">
		<xsl:apply-templates select="configtag"/>
	</table>
</xsl:template>

<xsl:template match="configtag">
	<xsl:param name="indent">0</xsl:param>
	<xsl:variable name="row-style"><xsl:if test="@manual">color: #62186f</xsl:if></xsl:variable>

	<xsl:choose>
		<xsl:when test="count(child::configtag)>0">
			<tr style="{$row-style}"><td style="text-indent:{$indent}pt">&lt;<xsl:value-of select="@name"/>&gt;</td></tr>
			<xsl:apply-templates select="configtag">
				<xsl:with-param name="indent"><xsl:value-of select="$indent+16"/></xsl:with-param>
			</xsl:apply-templates>
		</xsl:when>
		<xsl:otherwise>
			<tr style="{$row-style}"><td style="text-indent:{$indent}pt">&lt;<xsl:value-of select="@name"/>&gt;</td>
			<td rowspan="3"><xsl:apply-templates select="comment"/></td></tr>
			<tr style="{$row-style}"><td style="text-indent:{$indent}pt"><xsl:value-of select="text()"/></td></tr>
		</xsl:otherwise>
	</xsl:choose>
	<xsl:if test="@closing-tag='yes'">
		<tr style="{$row-style}"><td style="text-indent:{$indent}pt">&lt;/<xsl:value-of select="@name"/>&gt;</td></tr>
	</xsl:if>
</xsl:template>

<xsl:template match="comment">
	<xsl:apply-templates/>
</xsl:template>

</xsl:stylesheet>

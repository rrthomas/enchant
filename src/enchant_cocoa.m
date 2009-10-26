/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2004 Francis James Franklin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#import "enchant_cocoa.h"

static EnchantResourceProvider * s_instance = 0;

@implementation EnchantResourceProvider

+ (EnchantResourceProvider *)instance
{
	if (!s_instance)
		{
			s_instance = [[EnchantResourceProvider alloc] init];
		}
	return s_instance;
}

- (id)init
{
	if (self = [super init])
		{
			ModuleFolder = [[NSBundle bundleForClass:[self class]] resourcePath];
			[ModuleFolder retain];

			ConfigFolder = [[NSString alloc] initWithUTF8String:"/Library/Application Support/Enchant"];
		}
	return self;
}

- (void)dealloc
{
	if (ModuleFolder)
		{
			[ModuleFolder release];
			ModuleFolder = 0;
		}
	if (ConfigFolder)
		{
			[ConfigFolder release];
			ConfigFolder = 0;
		}
	[super dealloc];
}

- (const char *)moduleFolder
{
	return [ModuleFolder UTF8String];
}

- (const char *)configFolder
{
	return [ConfigFolder UTF8String];
}

@end

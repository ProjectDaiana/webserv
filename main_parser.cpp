/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 16:26:00 by ltreser           #+#    #+#             */
/*   Updated: 2025/08/30 19:30:35 by ltreser          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int	main(void)
{
	t_data *data = init_data();
	printf("data allocated\n");
	init_data();
	free_arena(data->perm_memory);
	printf("data freed\n");
	return (0);
}
